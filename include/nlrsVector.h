#pragma once

#include "nlrsAliases.h"
#include "nlrsAssert.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <type_traits>

namespace nlrs
{

namespace detail
{

template<typename T, bool IsFloatingPoint>
struct Number
{
    static bool hasNans(const T(&v)[2])
    {
        return std::isnan(v[0]) || std::isnan(v[1]);
    }

    static bool hasInfs(const T(&v)[2])
    {
        return std::isinf(v[0]) || std::isinf(v[1]);
    }

    static bool hasNans(const T(&v)[3])
    {
        return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]);
    }

    static bool hasInfs(const T(&v)[3])
    {
        return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]);
    }

    static bool hasNans(const T(&v)[4])
    {
        return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]) || std::isnan(v[3]);
    }

    static bool hasInfs(const T(&v)[4])
    {
        return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]) || std::isinf(v[3]);
    }
};

template<typename T>
struct Number<T, false>
{
    static bool hasNans(const T(&v)[2])
    {
        return false;
    }

    static bool hasInfs(const T(&v)[2])
    {
        return false;
    }

    static bool hasNans(const T(&v)[3])
    {
        return false;
    }

    static bool hasInfs(const T(&v)[3])
    {
        return false;
    }

    static bool hasNans(const T(&v)[4])
    {
        return false;
    }

    static bool hasInfs(const T(&v)[4])
    {
        return false;
    }
};

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
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
    }

    Vector2(const T(&array)[2])
        : data{ array[0], array[1] }
    {
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
    }

    Vector2(std::initializer_list<T> l)
        : x(T(0)),
        y(T(0))
    {
        NLRS_ASSERT(l.size() == 2u);
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
        u32 i = 0u;
        for (T t : l)
        {
            data[i] = t;
            ++i;
        }
    }

    template<typename D>
    Vector2<D> cast() const
    {
        return Vector2<D>{ D(x), D(y) };
    }

    Vector2 abs() const
    {
        return Vector2{ std::abs(x), std::abs(y) };
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
        return Vector2{ x*norm, y*norm };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
    }

    T dot(const Vector2& rhs) const
    {
        return x*rhs.x + y*rhs.y;
    }

    Vector2 hadamard(const Vector2& rhs) const
    {
        return Vector2{ x*rhs.x, y*rhs.y };
    }

    Vector2 operator+(const Vector2& rhs) const
    {
        return Vector2{ x + rhs.x, y + rhs.y };
    }

    Vector2 operator-(const Vector2& rhs) const
    {
        return Vector2{ x - rhs.x, y - rhs.y };
    }

    Vector2 operator-() const
    {
        return Vector2{ -x, -y };
    }

    Vector2 operator*(T val) const
    {
        return Vector2{ x*val, y*val };
    }

    Vector2 operator/(T val) const
    {
        return Vector2{ x / val, y / val };
    }

    Vector2& operator+=(const Vector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;

        return *this;
    }

    Vector2& operator-=(const Vector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;

        return *this;
    }

    Vector2& operator*=(T val)
    {
        x *= val;
        y *= val;

        return *this;
    }

    Vector2& operator/=(T val)
    {
        x /= val;
        y /= val;
    }

    bool operator==(const Vector2& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const Vector2& rhs) const
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
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
    }

    Vector3(const T(&array)[3])
        : data{ array[0], array[1], array[2] }
    {
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
    }

    Vector3(std::initializer_list<T> l)
        : x(T(0)),
        y(T(0)),
        z(T(0))
    {
        NLRS_ASSERT(l.size() == 3u);
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
        u32 i = 0u;
        for (T t : l)
        {
            data[i] = t;
            ++i;
        }
    }

    static Vector3 axisX()
    {
        return Vector3{ T(1), T(0), T(0) };
    }

    static Vector3 axisY()
    {
        return Vector3{ T(0), T(1), T(0) };
    }

    static Vector3 axisZ()
    {
        return Vector3{ T(0), T(0), T(1) };
    }

    template<typename D>
    Vector3<D> cast() const
    {
        return Vector3<D>{ D(x), D(y), D(z) };
    }

    Vector3 abs() const
    {
        return Vector3{ std::abs(x), std::abs(y), std::abs(z) };
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
        return Vector3{ x*n, y*n, z*n };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        x *= n;
        y *= n;
        z *= n;
    }

    T dot(const Vector3& rhs) const
    {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    Vector3 hadamard(const Vector3& rhs) const
    {
        return Vector3{ x*rhs.x, y*rhs.y, z*rhs.z };
    }

    Vector3 cross(const Vector3& rhs) const
    {
        return Vector3{
            y*rhs.z - z*rhs.y,
            z*rhs.x - x*rhs.z,
            x*rhs.y - y*rhs.x
        };
    }

    Vector3 operator+(const Vector3& rhs) const
    {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector3 operator-(const Vector3& rhs) const
    {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vector3 operator-() const
    {
        return Vector3{-x, -y, -z};
    }

    Vector3 operator*(T val) const
    {
        return Vector3{ x*val, y*val, z*val };
    }

    Vector3 operator/(T val) const
    {
        return Vector3{ x / val, y / val, z / val };
    }

    Vector3& operator+=(const Vector3& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;

        return *this;
    }

    Vector3& operator-=(const Vector3& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;

        return *this;
    }

    Vector3& operator*=(T val)
    {
        x *= val;
        y *= val;
        z *= val;

        return *this;
    }

    Vector3& operator/=(T val)
    {
        x /= val;
        y /= val;
        z /= val;

        return *this;
    }

    bool operator==(const Vector3& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    bool operator!=(const Vector3& rhs) const
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
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
    }

    Vector4(T x, T y, T z, T w)
        : data{ x, y, z, w }
    {
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
    }

    Vector4(const T(&array)[4])
        : data{ array[0], array[1], array[2], array[3] }
    {
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
    }

    Vector4(std::initializer_list<T> l)
        : x(T(0)),
        y(T(0)),
        z(T(0)),
        w(T(0))
    {
        NLRS_ASSERT(l.size() == 4u);
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(data)));
        u32 i = 0u;
        for (T t : l)
        {
            data[i] = t;
            ++i;
        }
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
        return Vector4{ std::abs(x), std::abs(y), std::abs(z), std::abs(w) };
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
        return Vector4{ x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w };
    }

    Vector4 operator+(const Vector4<T>& rhs) const
    {
        return Vector4{ x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
    }

    Vector4 operator-(const Vector4<T>& rhs) const
    {
        return Vector4{ x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
    }

    Vector4 operator-() const
    {
        return Vector4{ -x, -y, -z, -w };
    }

    Vector4 operator*(T val) const
    {
        return Vector4{ x*val, y*val, z*val, w*val };
    }

    Vector4 operator/(T val) const
    {
        return Vector4{ x / val, y / val, z / val, w / val };
    }

    Vector4& operator+=(const Vector4& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;

        return *this;
    }

    Vector4& operator-=(const Vector4& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;

        return *this;
    }

    Vector4& operator*=(T val)
    {
        x *= val;
        y *= val;
        z *= val;
        w *= val;

        return *this;
    }

    Vector4& operator/=(T val)
    {
        x /= val;
        y /= val;
        z /= val;
        w /= val;

        return *this;
    }

    bool operator==(const Vector4& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }

    bool operator!=(const Vector4& rhs) const
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
using Vec2i = Vector2<i32>;
using Vec3i = Vector3<i32>;
using Vec4i = Vector4<i32>;

}
