#pragma once

#include "aliases.h"
#include "locator.h"
#include "resizable_array.h"
#include "vector.h"

#include "stl/vector.h"

namespace nlrs
{

using buffer_handle = u64;
using descriptor_handle = uptr;
using shader_handle = u32;
using pipeline_handle = uptr;

/***
*       ___       ______
*      / _ )__ __/ _/ _/__ ____
*     / _  / // / _/ _/ -_) __/
*    /____/\_,_/_//_/ \__/_/
*
*/

enum class buffer_type
{
    array,
    index_array,
    uniform
};

enum class buffer_usage_hint
{
    constant, // the user will set the data once
    dynamic,  // the user will set the data occasionally
    stream    // the user will set the data every frame
};

struct buffer_options
{
    buffer_type       type;
    buffer_usage_hint hint;
};

enum class attribute_type
{
    float1,
    float2,
    float3,
    float4
};

struct vertex_attribute
{
    vertex_attribute(i32 location, attribute_type type)
        : used_(true),
        location_(location),
        type_(type)
    {}

    vertex_attribute(attribute_type type)
        : used_(false),
        location_(-1),
        type_(type)
    {}

    inline bool used() const { return used_; }
    inline i32 location() const { return location_; }
    inline attribute_type type() const { return type_; }

private:
    bool used_;
    i32 location_;
    attribute_type type_;
};

using descriptor_options = resizable_array<vertex_attribute, 6>;

/***
*       ______           __
*      / __/ /  ___ ____/ /__ ____
*     _\ \/ _ \/ _ `/ _  / -_) __/
*    /___/_//_/\_,_/\_,_/\__/_/
*
*/

enum class shader_type
{
    vertex,
    fragment,
    geometry,
    compute
};

struct uniform
{
    buffer_handle buffer;
    const char* blockName;
};

struct shader_stage
{
    shader_type type;
    const char* source;
    resizable_array<uniform, 6> uniforms;
};

/***
*       ___  _          ___
*      / _ \(_)__  ___ / (_)__  ___
*     / ___/ / _ \/ -_) / / _ \/ -_)
*    /_/  /_/ .__/\__/_/_/_//_/\__/
*          /_/
*/

// for OpenGL correspondance, see http://docs.gl/gl4/glDepthFunc
enum class comparison_function
{
    never,
    less,
    equal,
    lequal,
    greater,
    not_equal,
    gequal,
    always
};

// for OpenGL correspondance, see http://docs.gl/gl4/glBlendEquation
// TODO: see which one of these OpenGL uses by default
enum class blend_function
{
    add,
    subtract,
    reverse_subtract
};

struct pipeline_options
{
    pipeline_options(shader_handle shaderInfo)
        : shader(shaderInfo),
        depth_test_enabled(true),
        culling_enabled(true),
        scissor_test_enabled(false),
        blend_enabled(false),
        depth_comparison_func(comparison_function::less),
        blend_func(blend_function::add)   // TODO: see which one of these OpenGL uses by default
    {}

    shader_handle shader;
    bool depth_test_enabled;
    bool culling_enabled;
    bool scissor_test_enabled;
    bool blend_enabled;
    comparison_function depth_comparison_func;
    blend_function blend_func;
};

/***
*       ___                  ______       __
*      / _ \_______ __    __/ __/ /____ _/ /____
*     / // / __/ _ `/ |/|/ /\ \/ __/ _ `/ __/ -_)
*    /____/_/  \_,_/|__,__/___/\__/\_,_/\__/\__/
*
*/

enum class draw_mode
{
    triangle,
    point
};

enum class index_type
{
    ubyte,
    uint16,
    uint32
};

struct draw_state
{
    buffer_handle buffer;
    descriptor_handle descriptor;
    draw_mode mode;
    int index_count;
};

/***
*      _____              __   _           ___   ___  ____
*     / ___/______ ____  / /  (_)______   / _ | / _ \/  _/
*    / (_ / __/ _ `/ _ \/ _ \/ / __(_-<  / __ |/ ___// /
*    \___/_/  \_,_/ .__/_//_/_/\__/___/ /_/ |_/_/  /___/
*                /_/
*/

// TODO: pass window parameter explicitly
class graphics_api
{
public:
    static constexpr buffer_handle     invalid_buffer{ 0xffffffffffffffff };
    static constexpr descriptor_handle invalid_descriptor{ 0u };
    static constexpr shader_handle     invalid_shader{ 0u };
    static constexpr pipeline_handle   invalid_pipeline{ 0u };

    static constexpr usize max_pipelines{ 32u };
    static constexpr usize max_descriptors{ 32u };

    struct pass_options
    {
        // TODO: render target
        vec3f clearColor{ 0.f, 0.f, 0.f };
    };

    struct options
    {
        int major{ 3 };
        int minor{ 3 };
        int depth_bits{ 24 };
        int stencil_bits{ 8 };
        int ms_buffers{ 1 };
        int sample_count{ 4 };
    };

    graphics_api();
    ~graphics_api();

    graphics_api(const graphics_api&) = delete;
    graphics_api(graphics_api&&) = delete;
    graphics_api& operator=(const graphics_api&) = delete;
    graphics_api& operator=(graphics_api&&) = delete;

    bool initialize(const options& options);

    // create a new buffer on the GPU
    // data is a pointer to a contiguous array of data
    // elementSize is the size of each element in bytes, elementCount is the number of elements
    buffer_handle make_buffer_with_data(const buffer_options& options, const void* data, usize data_size);
    void set_buffer_data(buffer_handle info, const void* data, usize bytes);
    template<typename T>
    buffer_handle make_buffer(const buffer_options& opts, const std::pmr::vector<T>& data)
    {
        NLRS_ASSERT(data.size() != 0u);
        return make_buffer_with_data(opts, data.data(), sizeof(T) * data.size());
    }
    template<typename T>
    buffer_handle make_buffer(const buffer_options& opts, const T& obj)
    {
        return make_buffer_with_data(opts, &obj, sizeof(T));
    }
    template<typename T>
    void set_buffer(buffer_handle info, const std::pmr::vector<T>& data)
    {
        set_buffer_data(info, data.data(), data.size() * sizeof(T));
    }
    template<typename T>
    void set_buffer(buffer_handle info, const T& obj)
    {
        set_buffer_data(info, &obj, sizeof(T));
    }
    // release a buffer object created with make_buffer
    // if the buffer object is invalid, then this does nothing
    void release_buffer(buffer_handle info);

    // Use a descriptor object to specify the layout of the vertex data in a buffer
    descriptor_handle make_descriptor(const descriptor_options& attributes);
    void release_descriptor(descriptor_handle info);

    shader_handle make_shader(const std::pmr::vector<shader_stage>&);
    // release a shader created with make_shader
    // if the shader is invalid, this does nothing
    void release_shader(shader_handle info);

    pipeline_handle make_pipeline(const pipeline_options& opts);
    void release_pipeline(pipeline_handle);

    // TODO: should the render pass be tied to the concept of a pipeline or not?
    void begin_pass(pipeline_handle info);
    void end_pass();

    // render a buffer object
    // this must be called after begin_pass
    void apply_draw_state(const draw_state& draw_state);
    void apply_indexed_draw_state(const draw_state& draw_state, buffer_handle indices, index_type index_type);

    // TODO: this will probably be included in a draw pass
    void clear_buffers();

    // TODO: does this just use the window ptr?
    void swap_buffers();

private:
    struct RenderState;

    RenderState*    state_;
};

using graphics_api_locator = locator<graphics_api>;

}
