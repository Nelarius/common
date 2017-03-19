#include "aliases.h"
#include "memory_arena.h"
#include "configuration.h"
#include "graphics_api.h"
#include "log.h"
#include "object_pool.h"
#include "sdl_window.h"
#include "SDL_video.h"

// gl3w.h includes glcorearb.h, which includes the dreaded windows.h
#define NOMINMAX
#include "GL/gl3w.h"
#undef near
#undef far
#undef draw_state
#include "stl/vector.h"
#include "stl/unordered_map.h"
#include <cstring>
#include <string>
#include <utility>

namespace
{

// see the OpenGL types here:
// https://www.opengl.org/wiki/OpenGL_Type

struct GlBufferObject
{
    nlrs::u32 buffer;
    nlrs::i32 target;

    operator nlrs::buffer_handle() const
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
    const void* offset;
};

using GlDescriptor = nlrs::resizable_array<GlAttribute, 6>;

struct PipelineObject
{
    nlrs::shader_handle shader;
    bool depthTestEnabled;
    bool cullingEnabled;
    bool scissorTestEnabled;
    bool blendEnabled;
    nlrs::comparison_function depthComparisonFunction;
    nlrs::blend_function blendFunction;
};

struct RenderPass
{
    const PipelineObject* currentPipeline;
    GLint previousProgram;
    GLint previousVertexArrayObject;
    bool active;
};

static GlBufferObject& asGlBufferObject(nlrs::buffer_handle& info)
{
    return (GlBufferObject&)info;
}

static const GlBufferObject& asGlBufferObject(const nlrs::buffer_handle& info)
{
    return (const GlBufferObject&)info;
}

static PipelineObject& asPipelineObject(nlrs::pipeline_handle info)
{
    return *reinterpret_cast<PipelineObject*>(info);
}

static GlDescriptor& asDescriptorObject(nlrs::descriptor_handle info)
{
    return *reinterpret_cast<GlDescriptor*>(info);
}

static void applyDescriptor(nlrs::descriptor_handle info)
{
    const GlDescriptor& descriptor = asDescriptorObject(info);
    for (auto& attrib : descriptor)
    {
        glEnableVertexAttribArray(attrib.index);
        glVertexAttribPointer(
            attrib.index,
            attrib.elements,
            attrib.type,
            (GLboolean)GL_FALSE,
            attrib.stride,
            attrib.offset);
    }
}

GLenum asGlBufferTarget(nlrs::buffer_type type)
{
    switch (type)
    {
        case nlrs::buffer_type::array:       return GL_ARRAY_BUFFER;
        case nlrs::buffer_type::index_array: return GL_ELEMENT_ARRAY_BUFFER;
        case nlrs::buffer_type::uniform:     return GL_UNIFORM_BUFFER;
        default: NLRS_ASSERT(!"Unreachable"); return 0;
    }
}

GLenum asGlUsageHint(nlrs::buffer_usage_hint hint)
{
    // DRAW indicates that the user will write the data, but not read it
    // READ indicates that the user didn't write the data, but they will read it
    // COPY indicates that the user will not write or read the data

    // for now, just assume the user will write data and read it
    switch (hint)
    {
        case nlrs::buffer_usage_hint::constant:  return GL_STATIC_DRAW;
        case nlrs::buffer_usage_hint::dynamic:   return GL_DYNAMIC_DRAW;
        case nlrs::buffer_usage_hint::stream:    return GL_STREAM_DRAW;
    }
    NLRS_ASSERT(!"You shouldn't reach this!");
    return 0;
}

GLenum asGlshader_type(nlrs::shader_type type)
{
    switch (type)
    {
        case nlrs::shader_type::vertex:     return GL_VERTEX_SHADER;
        case nlrs::shader_type::fragment:   return GL_FRAGMENT_SHADER;
        case nlrs::shader_type::geometry:   return GL_GEOMETRY_SHADER;
        case nlrs::shader_type::compute:    return GL_COMPUTE_SHADER;
        default: NLRS_ASSERT(!"Unreachable"); return 0;
    }
}

nlrs::u32 asByteSize(nlrs::attribute_type type)
{
    switch (type)
    {
        case nlrs::attribute_type::float1:  return 4u;
        case nlrs::attribute_type::float2:  return 8u;
        case nlrs::attribute_type::float3:  return 12u;
        case nlrs::attribute_type::float4:  return 16u;

    }
    NLRS_ASSERT(!"You shouldn't reach this");
    return 0u;
}

GLint asGlAttributeElementCount(nlrs::attribute_type type)
{
    switch (type)
    {
        case nlrs::attribute_type::float1:  return 1;
        case nlrs::attribute_type::float2:  return 2;
        case nlrs::attribute_type::float3:  return 3;
        case nlrs::attribute_type::float4:  return 4;
        default:
            NLRS_ASSERT(!"You shouldn't reach this");
            break;
    }
    return 0;
}

GLenum asGlattribute_type(nlrs::attribute_type type)
{
    switch (type)
    {
        case nlrs::attribute_type::float1:
        case nlrs::attribute_type::float2:
        case nlrs::attribute_type::float3:
        case nlrs::attribute_type::float4:  return GL_FLOAT;
        default:
            NLRS_ASSERT(!"You shouldn't reach this");
            break;
    }
    return 0u;
}

GLenum asGlBlendMode(nlrs::blend_function function)
{
    switch (function)
    {
        case nlrs::blend_function::add: return GL_FUNC_ADD;
        case nlrs::blend_function::subtract: return GL_FUNC_SUBTRACT;
        case nlrs::blend_function::reverse_subtract: return GL_FUNC_REVERSE_SUBTRACT;
    }
    NLRS_ASSERT(!"You shouldn't reach this");
    return 0;
}

GLenum asGlDepthFunc(nlrs::comparison_function function)
{
    switch (function)
    {
        case nlrs::comparison_function::never: return GL_NEVER;
        case nlrs::comparison_function::less: return GL_LESS;
        case nlrs::comparison_function::equal: return GL_EQUAL;
        case nlrs::comparison_function::lequal: return GL_LEQUAL;
        case nlrs::comparison_function::greater: return GL_GREATER;
        case nlrs::comparison_function::not_equal: return GL_NOTEQUAL;
        case nlrs::comparison_function::gequal: return GL_GEQUAL;
        case nlrs::comparison_function::always: return GL_ALWAYS;
    }
    NLRS_ASSERT(!"You shouldn't reach this");
    return 0;
}

GLenum asGlDrawMode(nlrs::draw_mode mode)
{
    switch (mode)
    {
        case nlrs::draw_mode::point: return GL_POINTS;
        case nlrs::draw_mode::triangle: return GL_TRIANGLES;
    }
    NLRS_ASSERT(!"You shouldn't reach this");
    return 0;
}

GLenum asGlIndexType(nlrs::index_type type)
{
    switch (type)
    {
        case nlrs::index_type::ubyte: return GL_UNSIGNED_BYTE;
        case nlrs::index_type::uint16: return GL_UNSIGNED_SHORT;
        case nlrs::index_type::uint32: return GL_UNSIGNED_INT;
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

struct graphics_api::RenderState
{
    SDL_GLContext context;
    object_pool<PipelineObject, max_pipelines> pipelines;
    object_pool<GlDescriptor, max_descriptors> descriptors;
    std::pmr::unordered_map<buffer_handle, u32> boundUniformBuffers;
    RenderPass renderPass;
    u32 currentUniformBinding;
    u32 dummyVao;

    RenderState(memory_arena& allocator)
        : context(nullptr),
        pipelines(allocator),
        descriptors(allocator),
        boundUniformBuffers(),
        renderPass{ 0 },
        currentUniformBinding(0u),
        dummyVao(0u)
    {}

    ~RenderState() = default;
};

graphics_api::graphics_api()
    : state_(nullptr)
{
    // TODO: heap allocation required here?
    // could just use system allocator here
    state_ = new (free_list_locator::get()->allocate(sizeof(RenderState), alignof(RenderState))) RenderState{ *free_list_locator::get() };
}

graphics_api::~graphics_api()
{
    if (state_->context != nullptr)
    {
        SDL_GL_DeleteContext(state_->context);
    }

    glDeleteVertexArrays(1, &state_->dummyVao);

    state_->~RenderState();
    free_list_locator::get()->free(state_);
    state_ = nullptr;
}

bool graphics_api::initialize(const options& opts)
{
    NLRS_ASSERT(state_->context == nullptr);
#ifdef NLRS_DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, opts.major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, opts.minor);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, opts.depth_bits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, opts.stencil_bits);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, opts.ms_buffers);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, opts.sample_count);

    state_->context = SDL_GL_CreateContext(window_locator::get()->ptr());

    if (state_->context == NULL)
    {
        return false;
    }

    if (gl3wInit())
    {
        return false;
    }

    if (!gl3wIsSupported(opts.major, opts.minor))
    {
        return false;
    }

    LOG_INFO << "Graphics API implementation: OpenGL " << glGetString(GL_VERSION);
    LOG_INFO << "Vendor: " << glGetString(GL_VENDOR);
    LOG_INFO << "Renderer: " << glGetString(GL_RENDERER);

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

    glGenVertexArrays(1, &state_->dummyVao);

    return true;
}

buffer_handle graphics_api::make_buffer_with_data(const buffer_options& options, const void* data, usize dataSize)
{
    GlBufferObject object = { 0 };
    glGenBuffers(1, &object.buffer);
    object.target = asGlBufferTarget(options.type);

    GLint prev = 0;
    glGetIntegerv(getBindingTarget(object.target), &prev);

    glBindBuffer(object.target, object.buffer);
    glBufferData(object.target, dataSize, data, asGlUsageHint(options.hint));
    glBindBuffer(object.target, prev);

    return object;
}

void graphics_api::set_buffer_data(buffer_handle info, const void* data, usize bytes)
{
    GlBufferObject obj = asGlBufferObject(info);

    GLint prev = 0;
    glGetIntegerv(getBindingTarget(obj.target), &prev);
    glBindBuffer(obj.target, obj.buffer);

    glBindBuffer(obj.target, obj.buffer);
    void* deviceMem = glMapBuffer(obj.target, GL_WRITE_ONLY);
    std::memcpy(deviceMem, data, bytes);
    glUnmapBuffer(obj.target);
    glBindBuffer(obj.target, prev);
}

void graphics_api::release_buffer(buffer_handle bufferInfo)
{
    if (bufferInfo == invalid_buffer)
    {
        LOG_DEBUG << "Renderer> Attempted to release invalid buffer";
        return;
    }

    GlBufferObject obj = asGlBufferObject(bufferInfo);
    glDeleteBuffers(1, &obj.buffer);
}

descriptor_handle graphics_api::make_descriptor(const descriptor_options& attributes)
{
    i32 stride = 0u;
    for (const auto& attrib : attributes)
    {
        stride += asByteSize(attrib.type());
    }

    GlDescriptor* descriptor = state_->descriptors.create();

    uptr offset = 0u;
    for (const auto& attrib : attributes)
    {
        if (attrib.used())
        {
            descriptor->emplace_back(
                attrib.location(),                          // index
                asGlAttributeElementCount(attrib.type()),   // elements
                asGlattribute_type(attrib.type()),           // type
                (GLboolean)GL_FALSE,                        // normalized
                stride,                                     // stride
                (const void*)offset                         // offset
            );
        }
        offset += asByteSize(attrib.type());
    }

    return reinterpret_cast<uptr>(descriptor);
}

void graphics_api::release_descriptor(descriptor_handle info)
{
    state_->descriptors.release(&asDescriptorObject(info));
}

shader_handle graphics_api::make_shader(const std::pmr::vector<shader_stage>& stages)
{
    u32 program = glCreateProgram();

    if (program == 0u)
    {
        return invalid_shader;
    }

    for (auto stage : stages)
    {
        u32 shader = glCreateShader(asGlshader_type(stage.type));
        glShaderSource(shader, 1, &stage.source, 0);

        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE)
        {
            i32 infoLogLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
            // TODO: use a stack allocator for this?
            char* infoLogStr = (char*)system_memory_locator::get()->allocate(infoLogLength + 1);
            infoLogStr[infoLogLength] = '\0';
            glGetShaderInfoLog(shader, infoLogLength, 0, infoLogStr);
            LOG_ERROR << "Shader compilation failed: " << infoLogStr;
            system_memory_locator::get()->free(infoLogStr);
            glDeleteShader(shader);
            glDeleteProgram(program);

            return invalid_shader;
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
        char* infoLogStr = (char*)system_memory_locator::get()->allocate(infoLogLength + 1);
        glGetProgramInfoLog(program, infoLogLength, 0, infoLogStr);
        system_memory_locator::get()->free(infoLogStr);
        glDeleteProgram(program);

        return invalid_shader;
    }

    // set the uniform block buffer bindings if the program linkage succeeded
    for (const auto& stage : stages)
    {
        for (const auto& u : stage.uniforms)
        {
            const GlBufferObject& obj = asGlBufferObject(u.buffer);

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
            glUniformBlockBinding(program, glGetUniformBlockIndex(program, u.blockName), bufferBinding);
        }
    }

    return program;
}

void graphics_api::release_shader(shader_handle program)
{
    if (program == invalid_shader)
    {
        LOG_DEBUG << "Renderer> Attempted to release invalid shader";
        return;
    }
    glDeleteProgram(program);
}

pipeline_handle graphics_api::make_pipeline(const pipeline_options& opts)
{
    // TODO: give PipelineObject a constructor for PipelineOptions
    PipelineObject* obj = state_->pipelines.create(
        opts.shader, opts.depth_test_enabled, opts.culling_enabled,
        opts.scissor_test_enabled, opts.blend_enabled, opts.depth_comparison_func, opts.blend_func);

    NLRS_ASSERT(obj != nullptr);

    return reinterpret_cast<uptr>(obj);
}

void graphics_api::release_pipeline(pipeline_handle info)
{
    if (info == invalid_pipeline)
    {
        LOG_DEBUG << "Renderer> Attempted to release invalid pipeline object";
        return;
    }

    PipelineObject* obj = reinterpret_cast<PipelineObject*>(info);
    state_->pipelines.release(obj);
}

void graphics_api::begin_pass(pipeline_handle info)
{
    const PipelineObject& pipeline = asPipelineObject(info);

    state_->renderPass.active = true;
    state_->renderPass.currentPipeline = &pipeline;
    glGetIntegerv(GL_CURRENT_PROGRAM, &state_->renderPass.previousProgram);
    NLRS_ASSERT(state_->renderPass.previousProgram >= 0);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &state_->renderPass.previousVertexArrayObject);
    NLRS_ASSERT(state_->renderPass.previousVertexArrayObject >= 0);

    glUseProgram(pipeline.shader);
    glBindVertexArray(state_->dummyVao);
}

void graphics_api::end_pass()
{
    NLRS_ASSERT(state_->renderPass.active);

    glUseProgram(state_->renderPass.previousProgram);
    glBindVertexArray(state_->renderPass.previousVertexArrayObject);

    state_->renderPass.active = false;
}

void graphics_api::apply_draw_state(const draw_state& state)
{
    NLRS_ASSERT(state_->renderPass.active);

    const GlBufferObject& obj = asGlBufferObject(state.buffer);
    glBindBuffer(obj.target, obj.buffer);
    applyDescriptor(state.descriptor);
    glDrawArrays(asGlDrawMode(state.mode), 0, state.index_count);
}

void graphics_api::apply_indexed_draw_state(const draw_state& state, buffer_handle ihandle, index_type itype)
{
    NLRS_ASSERT(state_->renderPass.active);

    const GlBufferObject& verts = asGlBufferObject(state.buffer);
    const GlBufferObject& indices = asGlBufferObject(ihandle);
    glBindBuffer(verts.target, verts.buffer);
    glBindBuffer(indices.target, indices.buffer);
    applyDescriptor(state.descriptor);
    glDrawElements(asGlDrawMode(state.mode), state.index_count, asGlIndexType(itype), nullptr);
}

void graphics_api::clear_buffers()
{
    NLRS_ASSERT(!state_->renderPass.active);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void graphics_api::swap_buffers()
{
    NLRS_ASSERT(!state_->renderPass.active);
    SDL_GL_SwapWindow(window_locator::get()->ptr());
}

}
