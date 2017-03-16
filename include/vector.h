#pragma once

#include "aliases.h"
#include "nlrs_assert.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <type_traits>

namespace nlrs
{

namespace detail
{

template<typename T, bool IsFloatingPoint>
struct number
{
    static bool has_nans(const T(&v)[2])
    {
        return std::isnan(v[0]) || std::isnan(v[1]);
    }

    static bool has_infs(const T(&v)[2])
    {
        return std::isinf(v[0]) || std::isinf(v[1]);
    }

    static bool has_nans(const T(&v)[3])
    {
        return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]);
    }

    static bool has_infs(const T(&v)[3])
    {
        return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]);
    }

    static bool has_nans(const T(&v)[4])
    {
        return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]) || std::isnan(v[3]);
    }

    static bool has_infs(const T(&v)[4])
    {
        return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]) || std::isinf(v[3]);
    }
};

template<typename T>
struct number<T, false>
{
    static bool has_nans(const T(&v)[2])
    {
        return false;
    }

    static bool has_infs(const T(&v)[2])
    {
        return false;
    }

    static bool has_nans(const T(&v)[3])
    {
        return false;
    }

    static bool has_infs(const T(&v)[3])
    {
        return false;
    }

    static bool has_nans(const T(&v)[4])
    {
        return false;
    }

    static bool has_infs(const T(&v)[4])
    {
        return false;
    }
};

} // detail

template<typename T>
struct vector2
{
    vector2()
        : data{ T(0.0), T(0.0) }
    {}

    vector2(T x, T y)
        : data{ x, y }
    {
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    vector2(const T(&array)[2])
        : data{ array[0], array[1] }
    {
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    vector2(std::initializer_list<T> l)
        : x(T(0)),
        y(T(0))
    {
        NLRS_ASSERT(l.size() == 2u);
        u32 i = 0u;
        for (T t : l)
        {
            data[i] = t;
            ++i;
        }
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    template<typename D>
    vector2<D> cast() const
    {
        return vector2<D>{ D(x), D(y) };
    }

    vector2 abs() const
    {
        return vector2{ std::abs(x), std::abs(y) };
    }

    T min() const
    {
        return std::min(x, y);
    }

    T max() const
    {
        return std::max(x, y);
    }

    T norm() const
    {
        return sqrt(x*x + y*y);
    }

    T normSquared() const
    {
        return x*x + y*y;
    }

    vector2 normalized() const
    {
        T norm = 1.0 / norm();
        return vector2{ x*norm, y*norm };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
    }

    T dot(const vector2& rhs) const
    {
        return x*rhs.x + y*rhs.y;
    }

    vector2 hadamard(const vector2& rhs) const
    {
        return vector2{ x*rhs.x, y*rhs.y };
    }

    vector2 operator+(const vector2& rhs) const
    {
        return vector2{ x + rhs.x, y + rhs.y };
    }

    vector2 operator-(const vector2& rhs) const
    {
        return vector2{ x - rhs.x, y - rhs.y };
    }

    vector2 operator-() const
    {
        return vector2{ -x, -y };
    }

    vector2 operator*(T val) const
    {
        return vector2{ x*val, y*val };
    }

    vector2 operator/(T val) const
    {
        return vector2{ x / val, y / val };
    }

    vector2& operator+=(const vector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;

        return *this;
    }

    vector2& operator-=(const vector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;

        return *this;
    }

    vector2& operator*=(T val)
    {
        x *= val;
        y *= val;

        return *this;
    }

    vector2& operator/=(T val)
    {
        x /= val;
        y /= val;
    }

    bool operator==(const vector2& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const vector2& rhs) const
    {
        return x != rhs.x || y != rhs.y;
    }

    T operator[](usize i) const
    {
        NLRS_ASSERT(i < 2u);
        return data[i];
    }

    union
    {
        T data[2];
        struct { T x, y; };
        struct { T r, g; };
    };
};

template<typename T>
vector2<T> operator*(T scale, const vector2<T>& rhs)
{
    return vector2<T>(
        rhs.data[0] * scale,
        rhs.data[1] * scale
        );
}

template<typename T>
struct vector3
{
    vector3()
        : data{ T(0.0), T(0.0), T(0.0) }
    {}

    vector3(T x, T y, T z)
        : data{ x, y, z }
    {
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    vector3(const T(&array)[3])
        : data{ array[0], array[1], array[2] }
    {
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    vector3(std::initializer_list<T> l)
        : x(T(0)),
        y(T(0)),
        z(T(0))
    {
        NLRS_ASSERT(l.size() == 3u);
        u32 i = 0u;
        for (T t : l)
        {
            data[i] = t;
            ++i;
        }
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    static vector3 axisX()
    {
        return vector3{ T(1), T(0), T(0) };
    }

    static vector3 axisY()
    {
        return vector3{ T(0), T(1), T(0) };
    }

    static vector3 axisZ()
    {
        return vector3{ T(0), T(0), T(1) };
    }

    template<typename D>
    vector3<D> cast() const
    {
        return vector3<D>{ D(x), D(y), D(z) };
    }

    vector3 abs() const
    {
        return vector3{ std::abs(x), std::abs(y), std::abs(z) };
    }

    T min() const
    {
        return std::min(x, std::min(y, z));
    }

    T max() const
    {
        return std::max(x, std::max(y, z));
    }

    T norm() const
    {
        return sqrt(x*x + y*y + z*z);
    }

    T normSquared() const
    {
        return x*x + y*y + z*z;
    }

    vector3 normalized() const
    {
        T n = T(1.0) / norm();
        return vector3{ x*n, y*n, z*n };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
        z *= n;
    }

    T dot(const vector3& rhs) const
    {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    vector3 hadamard(const vector3& rhs) const
    {
        return vector3{ x*rhs.x, y*rhs.y, z*rhs.z };
    }

    vector3 cross(const vector3& rhs) const
    {
        return vector3{
            y*rhs.z - z*rhs.y,
            z*rhs.x - x*rhs.z,
            x*rhs.y - y*rhs.x
        };
    }

    vector3 operator+(const vector3& rhs) const
    {
        return vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    vector3 operator-(const vector3& rhs) const
    {
        return vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    vector3 operator-() const
    {
        return vector3{-x, -y, -z};
    }

    vector3 operator*(T val) const
    {
        return vector3{ x*val, y*val, z*val };
    }

    vector3 operator/(T val) const
    {
        return vector3{ x / val, y / val, z / val };
    }

    vector3& operator+=(const vector3& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;

        return *this;
    }

    vector3& operator-=(const vector3& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;

        return *this;
    }

    vector3& operator*=(T val)
    {
        x *= val;
        y *= val;
        z *= val;

        return *this;
    }

    vector3& operator/=(T val)
    {
        x /= val;
        y /= val;
        z /= val;

        return *this;
    }

    bool operator==(const vector3& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool operator!=(const vector3& rhs) const
    {
        return x != rhs.x || y != rhs.y || z != rhs.z;
    }

    T operator[](usize i) const
    {
        NLRS_ASSERT(i < 3u);
        return data[i];
    }

    union
    {
        T data[3];
        struct { T x, y, z; };
        struct { T r, g, b; };
    };
};

template<typename T>
vector3<T> operator*(T scale, const vector3<T>& rhs)
{
    return vector3<T>{
        rhs.x * scale,
        rhs.y * scale,
        rhs.z * scale
    };
}

template<typename T>
struct vector4
{
    vector4()
        : data{ T(0.0), T(0.0), T(0.0), T(0.0) }
    {}

    vector4(const vector3<T>& v, T w = T(0.0))
        : data{ v.x, v.y, v.z, w }
    {
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    vector4(T x, T y, T z, T w)
        : data{ x, y, z, w }
    {
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    vector4(const T(&array)[4])
        : data{ array[0], array[1], array[2], array[3] }
    {
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    vector4(std::initializer_list<T> l)
        : x(T(0)),
        y(T(0)),
        z(T(0)),
        w(T(0))
    {
        NLRS_ASSERT(l.size() == 4u);
        u32 i = 0u;
        for (T t : l)
        {
            data[i] = t;
            ++i;
        }
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(data)));
    }

    template<typename D>
    vector4<D> cast() const
    {
        return vector4<D>{ D(x), D(y), D(z), D(w) };
    }

    operator vector3<T>() const
    {
        return vector3<T> { data[0], data[1], data[2] };
    }

    vector4 abs() const
    {
        return vector4{ std::abs(x), std::abs(y), std::abs(z), std::abs(w) };
    }

    T min() const
    {
        return std::min(std::min(x, y), std::min(z, w));
    }

    T max() const
    {
        return std::max(std::max(x, y), std::max(z, w));
    }

    T norm() const
    {
        return sqrt(x*x + y*y + z*z + w*w);
    }

    T normSquared() const
    {
        return x*x + y*y + z*z + w*w;
    }

    vector4 normalized() const
    {
        T norm = T(1.0) / norm();
        return vector4<T>{ x*norm, y*norm, z*norm, w*norm };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
        z *= n;
        w *= n;
    }

    T dot(const vector4<T>& rhs) const
    {
        return x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w;
    }

    vector4 hadamard(const vector4<T>& rhs) const
    {
        return vector4{ x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w };
    }

    vector4 operator+(const vector4<T>& rhs) const
    {
        return vector4{ x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
    }

    vector4 operator-(const vector4<T>& rhs) const
    {
        return vector4{ x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
    }

    vector4 operator-() const
    {
        return vector4{ -x, -y, -z, -w };
    }

    vector4 operator*(T val) const
    {
        return vector4{ x*val, y*val, z*val, w*val };
    }

    vector4 operator/(T val) const
    {
        return vector4{ x / val, y / val, z / val, w / val };
    }

    vector4& operator+=(const vector4& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;

        return *this;
    }

    vector4& operator-=(const vector4& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;

        return *this;
    }

    vector4& operator*=(T val)
    {
        x *= val;
        y *= val;
        z *= val;
        w *= val;

        return *this;
    }

    vector4& operator/=(T val)
    {
        x /= val;
        y /= val;
        z /= val;
        w /= val;

        return *this;
    }

    bool operator==(const vector4& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    bool operator!=(const vector4& rhs) const
    {
        return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
    }

    T operator[](usize i) const
    {
        NLRS_ASSERT(i < 4u);
        return data[i];
    }

    union
    {
        T data[4];
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
    };
};

template<typename T>
vector4<T> operator*(T scale, const vector4<T>& rhs)
{
    return vector4<T>{
        rhs.data[0] * scale,
        rhs.data[1] * scale,
        rhs.data[2] * scale,
        rhs.data[3] * scale
    };
}

using vec2f = vector2<float>;
using vec3f = vector3<float>;
using vec4f = vector4<float>;
using vec2d = vector2<double>;
using vec3d = vector3<double>;
using vec4d = vector4<double>;
using vec2i = vector2<i32>;
using vec3i = vector3<i32>;
using vec4i = vector4<i32>;

}
