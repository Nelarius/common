#pragma once

#include "nlrsAliases.h"
#include "nlrsArray.h"
#include "nlrsLocator.h"
#include "nlrsVector.h"

namespace nlrs
{

using BufferInfo = u64;
using DescriptorInfo = u32;
using ShaderInfo = u32;
using PipelineInfo = uptr;

/***
*       ___       ______
*      / _ )__ __/ _/ _/__ ____
*     / _  / // / _/ _/ -_) __/
*    /____/\_,_/_//_/ \__/_/
*
*/

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

enum class AttributeType
{
    Float1,
    Float2,
    Float3,
    Float4
};

struct VertexAttribute
{
    VertexAttribute(i32 location, AttributeType type)
        : used_(true),
        location_(location),
        type_(type)
    {}

    VertexAttribute(AttributeType type)
        : used_(false),
        location_(-1),
        type_(type)
    {}

    inline bool used() const { return used_; }
    inline i32 location() const { return location_; }
    inline AttributeType type() const { return type_; }

private:
    bool used_;
    i32 location_;
    AttributeType type_;
};

struct DescriptorOptions
{
    StaticArray<VertexAttribute, 6> attributes;
    StaticArray<BufferInfo, 6> buffers;
};

/***
*       ______           __
*      / __/ /  ___ ____/ /__ ____
*     _\ \/ _ \/ _ `/ _  / -_) __/
*    /___/_//_/\_,_/\_,_/\__/_/
*
*/

enum class ShaderType
{
    Vertex,
    Fragment,
    Geometry,
    Compute
};

struct Uniform
{
    BufferInfo buffer;
    u32 binding;
};

struct ShaderStage
{
    ShaderType type;
    const char* source;
    StaticArray<Uniform, 6> uniforms;
};

/***
*       ___  _          ___
*      / _ \(_)__  ___ / (_)__  ___
*     / ___/ / _ \/ -_) / / _ \/ -_)
*    /_/  /_/ .__/\__/_/_/_//_/\__/
*          /_/
*/

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
        depthTestEnabled(true),
        cullingEnabled(true),
        scissorTestEnabled(false),
        blendEnabled(false),
        depthComparisonFunction(ComparisonFunction::Less),
        blendFunction(BlendFunction::Add)   // TODO: see which one of these OpenGL uses by default
    {}

    ShaderInfo shader;
    bool depthTestEnabled;
    bool cullingEnabled;
    bool scissorTestEnabled;
    bool blendEnabled;
    ComparisonFunction depthComparisonFunction;
    BlendFunction blendFunction;
};

/***
*       ___                  ______       __
*      / _ \_______ __    __/ __/ /____ _/ /____
*     / // / __/ _ `/ |/|/ /\ \/ __/ _ `/ __/ -_)
*    /____/_/  \_,_/|__,__/___/\__/\_,_/\__/\__/
*
*/

enum class DrawMode
{
    Triangle,
    Point
};

enum class IndexType
{
    Ubyte,
    Uint16,
    Uint32
};

struct DrawState
{
    DescriptorInfo descriptor;
    DrawMode mode;
    int indexCount;
};

/***
*      _____              __   _           ___   ___  ____
*     / ___/______ ____  / /  (_)______   / _ | / _ \/  _/
*    / (_ / __/ _ `/ _ \/ _ \/ / __(_-<  / __ |/ ___// /
*    \___/_/  \_,_/ .__/_//_/_/\__/___/ /_/ |_/_/  /___/
*                /_/
*/

// TODO: pass window parameter explicitly
class GraphicsApi
{
public:
    static constexpr BufferInfo     InvalidBuffer{ 0xffffffffffffffff };
    static constexpr DescriptorInfo InvalidDescriptor{ 0u };
    static constexpr ShaderInfo     InvalidShader{ 0u };
    static constexpr PipelineInfo   InvalidPipeline{ 0u };

    static constexpr usize MaxPipelines{ 32u };
    static constexpr usize MaxDescriptors{ 32u };

    struct PassOptions
    {
        // TODO: render target
        Vec3f clearColor{ 0.f, 0.f, 0.f };
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
    BufferInfo makeBufferWithData(const BufferOptions& options, const void* data, usize dataSize);
    void setBufferData(BufferInfo info, const void* data, usize bytes);
    template<typename T>
    BufferInfo makeBuffer(const BufferOptions& opts, const Array<T>& data)
    {
        NLRS_ASSERT(data.size() != 0u);
        return makeBufferWithData(opts, data.data(), sizeof(T) * data.size());
    }
    template<typename T>
    BufferInfo makeBuffer(const BufferOptions& opts, const T& obj)
    {
        return makeBufferWithData(opts, &obj, sizeof(T));
    }
    template<typename T>
    void setBuffer(BufferInfo info, const Array<T>& data)
    {
        setBufferData(info, data.data(), data.size() * sizeof(T));
    }
    template<typename T>
    void setBuffer(BufferInfo info, const T& obj)
    {
        setBufferData(info, &obj, sizeof(T));
    }
    // release a buffer object created with makeBuffer
    // if the buffer object is invalid, then this does nothing
    void releaseBuffer(BufferInfo info);

    // Use a descriptor object to specify the layout of the vertex data in a buffer
    DescriptorInfo makeDescriptor(const DescriptorOptions& vertexDescriptor);
    void releaseDescriptor(DescriptorInfo info);

    ShaderInfo makeShader(const Array<ShaderStage>&);
    // release a shader created with makeShader
    // if the shader is invalid, this does nothing
    void releaseShader(ShaderInfo info);

    PipelineInfo makePipeline(const PipelineOptions& opts);
    void releasePipeline(PipelineInfo);

    // TODO: should the render pass be tied to the concept of a pipeline or not?
    void beginPass(PipelineInfo info);
    void endPass();

    // render a buffer object
    // this must be called after beginPass
    void applyDrawState(const DrawState& drawState);
    void applyIndexedDrawState(const DrawState& drawState, IndexType indexType);

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
