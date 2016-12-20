#pragma once

#include "nlrsAliases.h"
#include "nlrsAssert.h"
#include <algorithm>
#include <cmath>

namespace nlrs
{

namespace detail
{

template<typename T>
bool hasNans(const Vector2<T>& v)
{
    return std::isnan(v.x) || std::isnan(v.y);
}

template<typename T>
bool hasNans(const Vector3<T>& v)
{
    return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
}

template<typename T>
bool hasNans(const Vector4<T>& v)
{
    return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z) || std::isnan(v.w);
}

} // detail

template<typename T>
struct Vector2
{
    Vector2()
        : data{ T(0.0), T(0.0) }
    {}

    Vector2(T x, T y)
        : data{ x, y }
    {
        NLRS_ASSERT(!detail::hasNans(*this));
    }

    Vector2(const T(&array)[2])
        : data{ array[0], array[1] }
    {
        NLRS_ASSERT(!detail::hasNans(*this));
    }

    template<typename D>
    Vector2<D> cast() const
    {
        return Vector2<D>{ D(x), D(y) };
    }

    Vector2 abs() const
    {
        return Vector2<T>{ std::abs(x), std::abs(y) };
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

    Vector2 normalized() const
    {
        T norm = 1.0 / norm();
        return Vector2<T>{ x*norm, y*norm };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
    }

    T dot(const Vector2<T>& rhs) const
    {
        return x*rhs.x + y*rhs.y;
    }

    Vector2 hadamard(const Vector2<T>& rhs) const
    {
        return Vector2<T>{ x*rhs.x, y*rhs.y };
    }

    Vector2<T> operator+(const Vector2<T>& rhs) const
    {
        return Vector2<T>{ x + rhs.x, y + rhs.y };
    }

    Vector2<T> operator-(const Vector2<T>& rhs) const
    {
        return Vector2<T>{ x - rhs.x, y - rhs.y };
    }

    Vector2<T> operator*(T val) const
    {
        return Vector2<T>{ x*val, y*val };
    }

    Vector2<T> operator/(T val) const
    {
        return Vector2<T>{ x / val, y / val };
    }

    bool operator==(const Vector2<T>& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const Vector2<T>& rhs) const
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
Vector2<T> operator*(T scale, const Vector2<T>& rhs)
{
    return Vector2<T>(
        rhs.data[0] * scale,
        rhs.data[1] * scale
        );
}

template<typename T>
struct Vector3
{
    Vector3()
        : data{ T(0.0), T(0.0), T(0.0) }
    {}

    Vector3(T x, T y, T z)
        : data{ x, y, z }
    {
        NLRS_ASSERT(!detail::hasNans(*this));
    }

    Vector3(const T(&array)[3])
        : data{ array[0], array[1], array[2] }
    {
        NLRS_ASSERT(!detail::hasNans(*this));
    }

    template<typename D>
    Vector3<D> cast() const
    {
        return Vector3<D>{ D(x), D(y), D(z) };
    }

    Vector3 abs() const
    {
        return Vector3<T>{ std::abs(x), std::abs(y), std::abs(z) };
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

    Vector3 normalized() const
    {
        T n = T(1.0) / norm();
        return Vector3<T>{ x*n, y*n, z*n };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
        z *= n;
    }

    T dot(const Vector3<T>& rhs) const
    {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    Vector3 hadamard(const Vector3<T>& rhs) const
    {
        return Vector3<T>{ x*rhs.x, y*rhs.y, z*rhs.z };
    }

    Vector3 cross(const Vector3& rhs) const
    {
        return Vector3{
            y*rhs.z - z*rhs.y,
            z*rhs.x - x*rhs.z,
            x*rhs.y - y*rhs.x
        };
    }

    Vector3 operator+(const Vector3<T>& rhs) const
    {
        return Vector3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector3 operator-(const Vector3<T>& rhs) const
    {
        return Vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vector3 operator*(T val) const
    {
        return Vector3<T>{ x*val, y*val, z*val };
    }

    Vector3 operator/(T val) const
    {
        return Vector3<T>{ x / val, y / val, z / val };
    }

    bool operator==(const Vector3<T>& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool operator!=(const Vector3<T>& rhs) const
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
Vector3<T> operator*(T scale, const Vector3<T>& rhs)
{
    return Vector3<T>{
        rhs.x * scale,
            rhs.y * scale,
            rhs.z * scale
    };
}

template<typename T>
struct Vector4
{
    Vector4()
        : data{ T(0.0), T(0.0), T(0.0), T(0.0) }
    {}

    Vector4(const Vector3<T>& v, T w = T(0.0))
        : data{ v.x, v.y, v.z, w }
    {
        NLRS_ASSERT(!detail::hasNans(*this));
    }

    Vector4(T x, T y, T z, T w)
        : data{ x, y, z, w }
    {
        NLRS_ASSERT(!detail::hasNans(*this));
    }

    Vector4(const T(&array)[4])
        : data{ array[0], array[1], array[2], array[3] }
    {
        NLRS_ASSERT(!detail::hasNans(*this));
    }

    template<typename D>
    Vector4<D> cast() const
    {
        return Vector4<D>{ D(x), D(y), D(z), D(w) };
    }

    operator Vector3<T>() const
    {
        return Vector3<T> { data[0], data[1], data[2] };
    }

    Vector4 abs() const
    {
        return Vector4<T>{ std::abs(x), std::abs(y), std::abs(z), std::abs(w) };
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

    Vector4 normalized() const
    {
        T norm = T(1.0) / norm();
        return Vector4<T>{ x*norm, y*norm, z*norm, w*norm };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
        z *= n;
        w *= n;
    }

    T dot(const Vector4<T>& rhs) const
    {
        return x*rhs.x + y*rhs.y + z*rhs.z + w*rhs.w;
    }

    Vector4 hadamard(const Vector4<T>& rhs) const
    {
        return Vector4<T>{ x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w };
    }

    Vector4 operator+(const Vector4<T>& rhs) const
    {
        return Vector4<T>{ x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
    }

    Vector4 operator-(const Vector4<T>& rhs) const
    {
        return Vector4<T>{ x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
    }

    Vector4 operator*(T val) const
    {
        return Vector4<T>{ x*val, y*val, z*val, w*val };
    }

    Vector4 operator/(T val) const
    {
        return Vector4<T>{ x / val, y / val, z / val, w / val };
    }

    bool operator==(const Vector4<T>& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    bool operator!=(const Vector4<T>& rhs) const
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
Vector4<T> operator*(T scale, const Vector4<T>& rhs)
{
    return Vector4<T>{
        rhs.data[0] * scale,
            rhs.data[1] * scale,
            rhs.data[2] * scale,
            rhs.data[3] * scale
    };
}

using Vec2f = Vector2<float>;
using Vec3f = Vector3<float>;
using Vec4f = Vector4<float>;
using Vec2d = Vector2<double>;
using Vec3d = Vector3<double>;
using Vec4d = Vector4<double>;
using Vec2i = Vector2<int>;
using Vec3i = Vector3<int>;
using Vec4i = Vector4<int>;

}
