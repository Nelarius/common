#pragma once

#include "nlrsAliases.h"
#include "nlrsArray.h"
#include "nlrsLocator.h"
#include "nlrsVector.h"

namespace nlrs
{

/*
 * The renderer uses SDLWindow, which has to be set via the WindowLocator.
 */
class Renderer
{
public:
    using BufferInfo = u64;
    using ShaderInfo = u32;
    using PipelineInfo = usize;

    static constexpr BufferInfo     InvalidBuffer{ 0xffffffffffffffff };
    static constexpr ShaderInfo     InvalidShader{ 0u };
    static constexpr PipelineInfo   InvalidPipeline{ 0xffffffffffffffff };

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

    struct PipelineOptions
    {
        ShaderInfo                       shader;
        StaticArray<VertexAttribute, 6>  layout;

        // TODO: other options such as
        // blend function
        // depth test
        // culling enabled
        // scissor test enabled
        // depth comparison function
    };

    struct Options
    {
        int depthBits{ 24 };
        int stencilBits{ 8 };
        int msBuffers{ 1 };
        int msSamples{ 4 };
    };

    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    bool initialize(const Options& options);

    template<typename T>
    BufferInfo makeBuffer(const BufferOptions& opts, const Array<T>& data)
    {
        NLRS_ASSERT(data.size() != 0u);
        return makeBufferWithData_(opts, data.data(), sizeof(T), data.size());
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

    // TODO: this will probably be included in a draw pass
    void clearBuffers();

    // TODO: does this just use the window ptr?
    void swapBuffers();

private:
    BufferInfo makeBufferWithData_(const BufferOptions& options, const void* data, usize elementSize, usize elementCount);

    struct RenderState;

    RenderState*    state_;
};

using RendererLocator = Locator<Renderer>;

}
