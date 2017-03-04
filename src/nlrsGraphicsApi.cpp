#include "nlrsGraphicsApi.h"
#include "nlrsWindow.h"
#include "nlrsAliases.h"
#include "nlrsAllocator.h"
#include "nlrsArray.h"
#include "nlrsConfiguration.h"
#include "nlrsLog.h"
#include "nlrsObjectPool.h"
#include "SDL_video.h"
#include "GL/gl3w.h"

#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>

namespace
{

// see the OpenGL types here:
// https://www.opengl.org/wiki/OpenGL_Type

struct GlBufferObject
{
    nlrs::u32 buffer;
    nlrs::i32 target;

    operator nlrs::BufferInfo() const
    {
        return *reinterpret_cast<const nlrs::u64*>(this);
    }
};

struct GlAttribute
{
    GLint       index;
    GLint       elements;
    GLenum      type;
    GLboolean   normalized;
    GLsizei     stride;
};

struct PipelineObject
{
    nlrs::ShaderInfo shader;
    nlrs::Array<GlAttribute> layout;
    bool depthTestEnabled;
    bool cullingEnabled;
    bool scissorTestEnabled;
    bool blendEnabled;
    nlrs::ComparisonFunction depthComparisonFunction;
    nlrs::BlendFunction blendFunction;
};

struct RenderPass
{
    GLuint currentProgram;
    GLint previousProgram;
    bool active;
};

GLenum asGlBufferTarget(nlrs::BufferType type)
{
    switch (type)
    {
        case nlrs::BufferType::Array:      return GL_ARRAY_BUFFER;
        case nlrs::BufferType::IndexArray: return GL_ELEMENT_ARRAY_BUFFER;
        case nlrs::BufferType::Uniform:    return GL_UNIFORM_BUFFER;
        default: NLRS_ASSERT(!"Unreachable"); return 0;
    }
}

GLenum asGlUsageHint(nlrs::BufferUsageHint hint)
{
    switch (hint)
    {
        case nlrs::BufferUsageHint::StaticDraw:    return GL_STATIC_DRAW;
        default: NLRS_ASSERT(!"Unreachable");                return 0;
    }
}

GLenum asGlShaderType(nlrs::ShaderType type)
{
    switch (type)
    {
        case nlrs::ShaderType::Vertex:     return GL_VERTEX_SHADER;
        case nlrs::ShaderType::Fragment:   return GL_FRAGMENT_SHADER;
        case nlrs::ShaderType::Geometry:   return GL_GEOMETRY_SHADER;
        case nlrs::ShaderType::Compute:    return GL_COMPUTE_SHADER;
        default: NLRS_ASSERT(!"Unreachable"); return 0;
    }
}

nlrs::u32 asByteSize(nlrs::AttributeType type)
{
    switch (type)
    {
        case nlrs::AttributeType::Float1:  return 4u;
        case nlrs::AttributeType::Float2:  return 8u;
        case nlrs::AttributeType::Float3:  return 12u;
        case nlrs::AttributeType::Float4:  return 16u;
        default:
            NLRS_ASSERT(!"You shouldn't reach this");
            break;
    }
    return 0u;
}

GLint asGlAttributeElementCount(nlrs::AttributeType type)
{
    switch (type)
    {
        case nlrs::AttributeType::Float1:  return 1;
        case nlrs::AttributeType::Float2:  return 2;
        case nlrs::AttributeType::Float3:  return 3;
        case nlrs::AttributeType::Float4:  return 4;
        default:
            NLRS_ASSERT(!"You shouldn't reach this");
            break;
    }
    return 0;
}

GLenum asGlAttributeType(nlrs::AttributeType type)
{
    switch (type)
    {
        case nlrs::AttributeType::Float1:
        case nlrs::AttributeType::Float2:
        case nlrs::AttributeType::Float3:
        case nlrs::AttributeType::Float4:  return GL_FLOAT;
        default:
            NLRS_ASSERT(!"You shouldn't reach this");
            break;
    }
    return 0u;
}

GLenum asGlBlendMode(nlrs::BlendFunction function)
{
    switch(function)
    {
        case nlrs::BlendFunction::Add: return GL_FUNC_ADD;
        case nlrs::BlendFunction::Subtract: return GL_FUNC_SUBTRACT;
        case nlrs::BlendFunction::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
    }
    NLRS_ASSERT(!"You shouldn't reach this");
    return 0;
}

GLenum asGlDepthFunc(nlrs::ComparisonFunction function)
{
    switch(function)
    {
        case nlrs::ComparisonFunction::Never: return GL_NEVER;
        case nlrs::ComparisonFunction::Less: return GL_LESS;
        case nlrs::ComparisonFunction::Equal: return GL_EQUAL;
        case nlrs::ComparisonFunction::Lequal: return GL_LEQUAL;
        case nlrs::ComparisonFunction::Greater: return GL_GREATER;
        case nlrs::ComparisonFunction::NotEqual: return GL_NOTEQUAL;
        case nlrs::ComparisonFunction::Gequal: return GL_GEQUAL;
        case nlrs::ComparisonFunction::Always: return GL_ALWAYS;
    }
    NLRS_ASSERT(!"You shouldn't reach this");
    return 0;
}

GLenum getBindingTarget(GLenum type)
{
    switch (type)
    {
        case GL_ARRAY_BUFFER:               return GL_ARRAY_BUFFER_BINDING;
        case GL_ELEMENT_ARRAY_BUFFER:       return GL_ELEMENT_ARRAY_BUFFER_BINDING;
            // TODO: add support for these?
            //case GL_COPY_READ_BUFFER:           return GL_COPY_READ_BUFFER_BINDING;
            //case GL_COPY_WRITE_BUFFER:          return GL_COPY_WRITE_BUFFER_BINDING;
        case GL_PIXEL_UNPACK_BUFFER:        return GL_PIXEL_UNPACK_BUFFER_BINDING;
        case GL_PIXEL_PACK_BUFFER:          return GL_PIXEL_PACK_BUFFER_BINDING;
        case GL_QUERY_BUFFER:               return GL_QUERY_BUFFER_BINDING;
        case GL_TRANSFORM_FEEDBACK_BUFFER:  return GL_TRANSFORM_FEEDBACK_BINDING;
        case GL_UNIFORM_BUFFER:             return GL_UNIFORM_BUFFER_BINDING;
        case GL_DRAW_INDIRECT_BUFFER:       return GL_DRAW_INDIRECT_BUFFER_BINDING;
        case GL_ATOMIC_COUNTER_BUFFER:      return GL_ATOMIC_COUNTER_BUFFER_BINDING;
        case GL_DISPATCH_INDIRECT_BUFFER:   return GL_DISPATCH_INDIRECT_BUFFER_BINDING;
        case GL_SHADER_STORAGE_BUFFER:      return GL_SHADER_STORAGE_BUFFER_BINDING;
        case GL_TEXTURE_BUFFER:             return GL_TEXTURE_BINDING_BUFFER;
        case GL_TEXTURE_1D:                 return GL_TEXTURE_BINDING_1D;
        case GL_TEXTURE_2D:                 return GL_TEXTURE_BINDING_2D;
        case GL_TEXTURE_3D:                 return GL_TEXTURE_BINDING_3D;
        case GL_TEXTURE_1D_ARRAY:           return GL_TEXTURE_BINDING_1D_ARRAY;
        case GL_TEXTURE_2D_ARRAY:           return GL_TEXTURE_BINDING_2D_ARRAY;
        case GL_TEXTURE_RECTANGLE:          return GL_TEXTURE_BINDING_RECTANGLE;
        case GL_TEXTURE_CUBE_MAP:           return GL_TEXTURE_BINDING_CUBE_MAP;
        case GL_TEXTURE_CUBE_MAP_ARRAY:     return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
        case GL_TEXTURE_2D_MULTISAMPLE:     return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:   return GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY;
        case GL_FRAMEBUFFER:                return GL_FRAMEBUFFER_BINDING;
        case GL_DRAW_FRAMEBUFFER:           return GL_DRAW_FRAMEBUFFER_BINDING;
        case GL_READ_FRAMEBUFFER:           return GL_READ_FRAMEBUFFER_BINDING;
        default:    NLRS_ASSERT(!"Attempted to query unsupported binding target!"); return 0;
    }
}

#ifdef NLRS_DEBUG
void debugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    std::string debugTypeStr{};
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            debugTypeStr = "GL_DEBUG_TYPE_ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            debugTypeStr = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            debugTypeStr = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            debugTypeStr = "GL_DEBUG_TYPE_PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            debugTypeStr = "GL_DEBUG_TYPE_PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_OTHER:
            debugTypeStr = "GL_DEBUG_TYPE_OTHER";
            break;
        case GL_DEBUG_TYPE_MARKER:
            debugTypeStr = "GL_DEBUG_TYPE_MARKER";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            debugTypeStr = "GL_DEBUG_TYPE_PUSH_GROUP";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            debugTypeStr = "GL_DEBUG_TYPE_POP_GROUP";
            break;
    }
    std::string debugSourceStr{};
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            debugSourceStr = "GL_DEBUG_SOURCE_API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            debugSourceStr = "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            debugSourceStr = "GL_DEBUG_SOURCE_SHADER_COMPILER";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            debugSourceStr = "GL_DEBUG_SOURCE_THIRD_PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            debugSourceStr = "GL_DEBUG_SOURCE_APPLICATION";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            debugSourceStr = "GL_DEBUG_SOURCE_OTHER";
            break;
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_LOW:
            LOG_WARNING << debugSourceStr << ", " << debugTypeStr << ": " << message;
            break;
    }
}
#endif

}

namespace nlrs
{

struct GraphicsApi::RenderState
{
    SDL_GLContext context;
    ObjectPool<PipelineObject> pipelines;
    std::unordered_map<BufferInfo, u32> boundUniformBuffers;
    RenderPass renderPass;
    u32 currentUniformBinding;

    RenderState(ObjectPool<PipelineObject, 32u>&& pipelinePool)
        : context(nullptr),
        pipelines(std::move(pipelinePool)),
        boundUniformBuffers(),
        renderPass{0u, 0, false},
        currentUniformBinding(0u)
    {}

    ~RenderState() = default;
};

GraphicsApi::GraphicsApi()
    : state_(nullptr)
{
    // TODO: heap allocation required here?
    // could just use system allocator here
    state_ = new (HeapAllocatorLocator::get()->allocate(sizeof(RenderState), alignof(RenderState)))
        RenderState{ ObjectPool<PipelineObject, 32u>(*HeapAllocatorLocator::get()) };
}

GraphicsApi::~GraphicsApi()
{
    if (state_->context != nullptr)
    {
        SDL_GL_DeleteContext(state_->context);
    }

    state_->~RenderState();
    HeapAllocatorLocator::get()->free(state_);
    state_ = nullptr;
}

bool GraphicsApi::initialize(const Options& opts)
{
    NLRS_ASSERT(state_->context == nullptr);
    const int glMajor = 3;
    const int glMinor = 3;
#ifdef NLRS_DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, opts.depthBits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, opts.stencilBits);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, opts.msBuffers);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, opts.msSamples);

    state_->context = SDL_GL_CreateContext(WindowLocator::get()->ptr());

    if (state_->context == NULL)
    {
        return false;
    }

    if (gl3wInit())
    {
        return false;
    }

    if (!gl3wIsSupported(glMajor, glMinor))
    {
        return false;
    }

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#ifdef NLRS_DEBUG
#if NLRS_OS == NLRS_WINDOWS
    // this is available only in OpenGL 4.3
    glDebugMessageCallback(&debugCallback, nullptr);
#endif
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0);

    return true;
}

BufferInfo GraphicsApi::makeBufferWithData(const BufferOptions& options, const void* data, usize elementSize, usize elementCount)
{
    GlBufferObject object = { 0 };
    glGenBuffers(1, &object.buffer);
    object.target = asGlBufferTarget(options.type);

    GLint prev = 0;
    glGetIntegerv(getBindingTarget(object.target), &prev);

    glBindBuffer(object.target, object.buffer);
    glBufferData(object.target, elementSize * elementCount, data, asGlUsageHint(options.hint));
    glBindBuffer(object.target, prev);

    return object;
}

void GraphicsApi::setBufferData(BufferInfo info, const void* data, usize bytes)
{
    GlBufferObject obj = *reinterpret_cast<const GlBufferObject*>(&info);

    glBindBuffer(obj.target, obj.buffer);
    GLint prev = 0;
    glGetIntegerv(getBindingTarget(obj.target), &prev);

    glBindBuffer(obj.target, obj.buffer);
    void* deviceMem = glMapBuffer(obj.target, GL_WRITE_ONLY);
    std::memcpy(deviceMem, data, bytes);
    glUnmapBuffer(obj.target);
    glBindBuffer(obj.target, prev);
}

void GraphicsApi::releaseBuffer(BufferInfo bufferInfo)
{
    if (bufferInfo == InvalidBuffer)
    {
        LOG_DEBUG << "Renderer> Attempted to release invalid buffer";
        return;
    }

    u32 object = reinterpret_cast<GlBufferObject*>(&bufferInfo)->buffer;
    glDeleteBuffers(1, &object);
}

ShaderInfo GraphicsApi::makeShader(const Array<ShaderStage>& stages)
{
    u32 program = glCreateProgram();

    if (program == 0u)
    {
        return InvalidShader;
    }

    for (auto stage : stages)
    {
        u32 shader = glCreateShader(asGlShaderType(stage.type));
        glShaderSource(shader, 1, &stage.source, 0);

        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE)
        {
            i32 infoLogLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
            // TODO: use a stack allocator for this?
            char* infoLogStr = (char*)SystemAllocatorLocator::get()->allocate(infoLogLength + 1);
            infoLogStr[infoLogLength] = '\0';
            glGetShaderInfoLog(shader, infoLogLength, 0, infoLogStr);
            LOG_ERROR << "Shader compilation failed: " << infoLogStr;
            SystemAllocatorLocator::get()->free(infoLogStr);
            glDeleteShader(shader);
            glDeleteProgram(program);

            return InvalidShader;
        }

        glAttachShader(program, shader);
        glDeleteShader(shader);
    }

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        i32 infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        // TODO: use a stack allocator for this?
        char* infoLogStr = (char*)SystemAllocatorLocator::get()->allocate(infoLogLength + 1);
        glGetProgramInfoLog(program, infoLogLength, 0, infoLogStr);
        SystemAllocatorLocator::get()->free(infoLogStr);
        glDeleteProgram(program);

        return InvalidShader;
    }

    // set the uniform block buffer bindings if the program linkage succeeded
    for (const auto& stage : stages)
    {
        for (const auto& u : stage.uniforms)
        {
            GlBufferObject obj = *reinterpret_cast<const GlBufferObject*>(&u.buffer);

            u32 bufferBinding;
            auto it = state_->boundUniformBuffers.find(u.buffer);
            if (it != state_->boundUniformBuffers.end())
            {
                bufferBinding = it->second;
            }
            else
            {
                bufferBinding = state_->currentUniformBinding++;
#ifdef NLRS_DEBUG
                i32 maxBindings;
                glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxBindings);
                NLRS_ASSERT(maxBindings != -1);
                NLRS_ASSERT(bufferBinding < u32(maxBindings));
#endif
                state_->boundUniformBuffers.insert(std::make_pair(u.buffer, bufferBinding));
                glBindBufferBase(obj.target, bufferBinding, obj.buffer);
            }
            glUniformBlockBinding(program, u.binding, bufferBinding);
        }
    }

    return program;
}

void GraphicsApi::releaseShader(ShaderInfo program)
{
    if (program == InvalidShader)
    {
        LOG_DEBUG << "Renderer> Attempted to release invalid shader";
        return;
    }
    glDeleteProgram(program);
}

PipelineInfo GraphicsApi::makePipeline(const PipelineOptions& opts)
{
    Array<GlAttribute> layout(*HeapAllocatorLocator::get());

    // build the stride by iterating over the element counts
    i32 stride = 0u;

    for (const auto& attrib : opts.layout)
    {
        stride += asByteSize(attrib.type);
    }

    for (const auto& attrib : opts.layout)
    {
        if (attrib.name)
        {
            layout.emplaceBack(
                glGetAttribLocation(opts.shader, attrib.name),
                asGlAttributeElementCount(attrib.type),
                asGlAttributeType(attrib.type),
                (GLboolean)GL_FALSE,
                stride
            );
        }
    }

    PipelineObject* obj = state_->pipelines.create(
        opts.shader, std::move(layout), opts.depthTestEnabled, opts.cullingEnabled,
        opts.scissorTestEnabled, opts.blendEnabled, opts.depthComparisonFunction, opts.blendFunction);

    NLRS_ASSERT(obj != nullptr);

    return reinterpret_cast<uptr>(obj);
}

void GraphicsApi::releasePipeline(PipelineInfo info)
{
    if (info == InvalidPipeline)
    {
        LOG_DEBUG << "Renderer> Attempted to release invalid pipeline object";
        return;
    }

    PipelineObject* obj = reinterpret_cast<PipelineObject*>(info);
    state_->pipelines.release(obj);
}

void GraphicsApi::beginPass(PipelineInfo info)
{
    PipelineObject& pipeline = *reinterpret_cast<PipelineObject*>(info);

    state_->renderPass.active = true;
    glGetIntegerv(GL_CURRENT_PROGRAM, &state_->renderPass.previousProgram);
    NLRS_ASSERT(state_->renderPass.previousProgram != -1);

    glUseProgram(pipeline.shader);

    for (const auto& layout : pipeline.layout)
    {
        glEnableVertexAttribArray(layout.index);
        glVertexAttribPointer(
            layout.index,      layout.elements, layout.type,
            layout.normalized, layout.stride,   0);
    }
}

void GraphicsApi::endPass()
{
    glUseProgram(state_->renderPass.previousProgram);
    state_->renderPass.active = true;
}

void GraphicsApi::clearBuffers()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsApi::swapBuffers()
{
    SDL_GL_SwapWindow(WindowLocator::get()->ptr());
}

}
