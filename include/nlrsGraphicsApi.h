#pragma once

#include "nlrsAliases.h"
#include "nlrsArray.h"
#include "nlrsLocator.h"
#include "nlrsVector.h"

namespace nlrs
{

// TODO: pass window parameter explicitly
class GraphicsApi
{
public:
    using BufferInfo = u64;
    using ShaderInfo = u32;
    using PipelineInfo = uptr;
    using DrawStateInfo = usize;

    static constexpr BufferInfo     InvalidBuffer{ 0xffffffffffffffff };
    static constexpr ShaderInfo     InvalidShader{ 0u };
    static constexpr PipelineInfo   InvalidPipeline{ 0u };

    enum class BufferType
    {
        Array,
        IndexArray,
        Uniform
    };

    enum class BufferUsageHint
    {
        StaticDraw
    };

    struct BufferOptions
    {
        BufferType      type;
        BufferUsageHint hint;
    };

    enum class ShaderType
    {
        Vertex,
        Fragment,
        Geometry,
        Compute
    };

    struct ShaderStage
    {
        ShaderType type;
        const char* source;
    };

    struct RenderPass
    {
        // TODO: render target
        Vec3f clearColor{ 0.f, 0.f, 0.f };
    };

    enum class AttributeType
    {
        Float1,
        Float2,
        Float3,
        Float4
    };

    struct VertexAttribute
    {
        VertexAttribute(const char* name, AttributeType type)
            : used(true),
            name(name),
            type(type)
        {}

        VertexAttribute(AttributeType type)
            : used(false),
            name(nullptr),
            type(type)
        {}

        bool            used;
        const char*     name;
        AttributeType   type;
    };

    // for OpenGL correspondance, see http://docs.gl/gl4/glDepthFunc
    enum class ComparisonFunction
    {
        Never,
        Less,
        Equal,
        Lequal,
        Greater,
        NotEqual,
        Gequal,
        Always
    };

    // for OpenGL correspondance, see http://docs.gl/gl4/glBlendEquation
    // TODO: see which one of these OpenGL uses by default
    enum class BlendFunction
    {
        Add,
        Subtract,
        ReverseSubtract
    };

    struct PipelineOptions
    {
        PipelineOptions(ShaderInfo shaderInfo)
            : shader(shaderInfo),
            layout(),
            depthTestEnabled(true),
            cullingEnabled(true),
            scissorTestEnabled(false),
            blendEnabled(false),
            depthComparisonFunction(ComparisonFunction::Less),
            blendFunction(BlendFunction::Add)   // TODO: see which one of these OpenGL uses by default
        {}

        ShaderInfo                       shader;
        StaticArray<VertexAttribute, 6>  layout;
        bool depthTestEnabled;
        bool cullingEnabled;
        bool scissorTestEnabled;
        bool blendEnabled;
        ComparisonFunction depthComparisonFunction;
        BlendFunction blendFunction;
    };

    struct DrawStateOptions
    {
        PipelineInfo pipeline;
        BufferInfo buffer;
    };

    struct Options
    {
        int depthBits{ 24 };
        int stencilBits{ 8 };
        int msBuffers{ 1 };
        int msSamples{ 4 };
    };

    GraphicsApi();
    ~GraphicsApi();

    GraphicsApi(const GraphicsApi&) = delete;
    GraphicsApi(GraphicsApi&&) = delete;
    GraphicsApi& operator=(const GraphicsApi&) = delete;
    GraphicsApi& operator=(GraphicsApi&&) = delete;

    bool initialize(const Options& options);

    // create a new buffer on the GPU
    // data is a pointer to a contiguous array of data
    // elementSize is the size of each element in bytes, elementCount is the number of elements
    BufferInfo makeBufferWithData(const BufferOptions& options, const void* data, usize elementSize, usize elementCount);
    template<typename T>
    BufferInfo makeBuffer(const BufferOptions& opts, const Array<T>& data)
    {
        NLRS_ASSERT(data.size() != 0u);
        return makeBufferWithData(opts, data.data(), sizeof(T), data.size());
    }
    // release a buffer object created with makeBuffer
    // if the buffer object is invalid, then this does nothing
    void releaseBuffer(BufferInfo info);

    // TODO: map buffer range
    // support should be added once you can create a buffer object with
    // uninitialized memory

    ShaderInfo makeShader(const Array<ShaderStage>&);
    // release a shader created with makeShader
    // if the shader is invalid, this does nothing
    void releaseShader(ShaderInfo info);

    PipelineInfo makePipeline(const PipelineOptions& opts);
    void releasePipeline(PipelineInfo);

    DrawStateInfo makeDrawState(const DrawStateOptions& opts);
    void releaseDrawState(DrawStateInfo);

    // TODO: this will probably be included in a draw pass
    void clearBuffers();

    // TODO: does this just use the window ptr?
    void swapBuffers();

private:
    struct RenderState;

    RenderState*    state_;
};

using GraphicsApiLocator = Locator<GraphicsApi>;

}
