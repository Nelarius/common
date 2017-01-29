#pragma once

#include "nlrsVector.h"
#include "nlrsMatrix.h"
#include <cmath>
#include <iterator>
#include <limits>

namespace nlrs
{

// The plane normal is calculated from the input points in a counter clockwise sense,
// according to the right hand rule.

template<typename T>
class Plane
{
public:
    Plane() = delete;
    Plane(const Vector3<T>& p0, const Vector3<T>& p1, const Vector3<T>& p2)
        : edge1_{ p1 - p0 },
        edge2_{ p2 - p0 },
        point_{ p0 },
        normal_{ edge1_.cross(edge2_).normalized() }
    {}

    const Vector3<T> point() const
    {
        return point_;
    }

    const Vector3<T> normal() const
    {
        return normal_;
    }

private:
    Vector3<T> edge1_;
    Vector3<T> edge2_;
    Vector3<T> point_;
    Vector3<T> normal_;
};

template<typename T>
struct Bounds2
{
    Vector2<T> min{ std::numeric_limits<T>::max(), std::numeric_limits<T>::max() };
    Vector2<T> max{ std::numeric_limits<T>::min(), std::numeric_limits<T>::min() };

    Bounds2() = default;
    Bounds2(const Vector2<T>& mn, const Vector2<T>& mx)
        : min(mn),
        max(mx)
    {}

    bool operator==(const Bounds2<T>& rhs) const
    {
        return min == rhs.min && max == rhs.max;
    }

    bool operator!=(const Bounds2<T>& rhs) const
    {
        return min != rhs.min && max != rhs.max;
    }

    Vector2<T> center() const
    {
        return T(0.5) * (min + max);
    }

    Vector2<T> extent() const
    {
        return max - min;
    }
};

template<typename T>
struct Direction
{
    Direction()
        : v(T(0.f), T(0.f), T(0.f), T(0.f))
    {}

    Direction(T x, T y, T z)
        : v(x, y, z, 0.f)
    {}

    Direction(const Vector3<T>& v3)
        : v(v3.x, v3.y, v3.z, 0.f)
    {}

    Direction(std::initializer_list<T> l)
        : v(0.f, 0.f, 0.f, 0.f)
    {
        NLRS_ASSERT(l.size() == 3u);
        u32 i = 0u;
        for (T t : l)
        {
            v.data[i] = t;
            ++i;
        }
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(v.data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(v.data)));
    }

    operator Vector3<T>() const
    {
        return d;
    }

    operator Vector4<T>() const
    {
        return v;
    }

    T dot(const Direction& rhs) const
    {
        return d.dot(rhs.d);
    }

    Direction cross(const Direction& rhs) const
    {
        return d.cross(rhs.d);
    }

    void normalize()
    {
        d.normalize();
    }

    T norm() const
    {
        return d.norm();
    }

    T normSquared() const
    {
        return d.normSquared();
    }

    Direction operator+(const Direction& rhs) const
    {
        return Direction(d + rhs.d);
    }

    Direction operator-(const Direction& rhs) const
    {
        return Direction(d - rhs.d);
    }

    Direction operator-() const
    {
        return Direction(-d);
    }

    Direction operator*(T val) const
    {
        return Direction(val * d);
    }

    Direction operator/(T val) const
    {
        return Direction(d / val);
    }

    Direction& operator+=(const Direction& rhs)
    {
        d += rhs.d;

        return *this;
    }

    Direction& operator-=(const Direction& rhs)
    {
        d -= rhs.d;

        return *this;
    }

    Direction& operator*=(T val)
    {
        d *= val;

        return *this;
    }

    Direction& operator/=(T val)
    {
        d /= val;

        return *this;
    }

    T operator[](usize i) const
    {
        return d[i];
    }

    union
    {
        Vector3<T> d;
        Vector4<T> v;
    };
};

template<typename T>
Direction<T> operator*(T val, const Direction<T>& rhs)
{
    return Direction<T>(val * rhs.d);
}

template<typename T>
Direction<T> operator*(const Matrix4<T>& m, const Direction<T>& d)
{
    return (Direction<T>&)m * d.v;
}

template<typename T>
struct Point
{
    Point()
        : v(T(0.f), T(0.f), T(0.f), T(1.f))
    {}

    Point(T x, T y, T z)
        : v(x, y, z, 1.f)
    {}

    Point(const Vector3<T>& v3)
        : v(v3.x, v3.y, v3.z, 1.f)
    {}

    Point(std::initializer_list<T> l)
        : v(0.f, 0.f, 0.f, 1.f)
    {
        NLRS_ASSERT(l.size() == 3u);
        u32 i = 0u;
        for (T t : l)
        {
            v.data[i] = t;
            ++i;
        }
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasNans(v.data)));
        NLRS_ASSERT(!(detail::Number<T, std::is_floating_point<T>::value>::hasInfs(v.data)));
    }

    operator Vector3<T>() const
    {
        return p;
    }

    operator Vector4<T>() const
    {
        return v;
    }

    T norm() const
    {
        return p.norm();
    }

    T normSquared() const
    {
        return p.normSquared();
    }

    Point operator+(const Point& rhs) const
    {
        return Point(p + rhs.p);
    }

    Point operator+(const Direction<T>& rhs) const
    {
        return Point(p + rhs.d);
    }

    Point operator-(const Point& rhs) const
    {
        return Point(p - rhs.p);
    }

    Point operator-(const Direction<T>& rhs) const
    {
        return Point(p - rhs.d);
    }

    Point operator-() const
    {
        return Point(-p);
    }

    Point operator*(T val) const
    {
        return Point(val * p);
    }

    Point operator/(T val) const
    {
        return Point(p / val);
    }

    Point& operator+=(const Point& rhs)
    {
        p += rhs.p;

        return *this;
    }

    Point& operator+=(const Direction<T>& rhs)
    {
        p += rhs.d;

        return *this;
    }

    Point& operator-=(const Point& rhs)
    {
        p -= rhs.p;

        return *this;
    }

    Point& operator-=(const Direction<T>& rhs)
    {
        p -= rhs.d;

        return *this;
    }

    Point& operator*=(T val)
    {
        p *= val;

        return *this;
    }

    Point& operator/=(T val)
    {
        p /= val;

        return *this;
    }

    T operator[](usize i) const
    {
        return p[i];
    }

    union
    {
        Vector3<T> p;
        Vector4<T> v;
    };
};

template<typename T>
Point<T> operator*(T val, const Point<T>& rhs)
{
    return Point<T>(val * rhs.p);
}

template<typename T>
Point<T> operator*(const Matrix4<T>& m, const Point<T>& p)
{

    return (Point<T>&)m * p.v;
}

using Planef = Plane<float>;
using Bounds2f = Bounds2<float>;
using Bounds2i = Bounds2<i32>;
using Pointf = Point<float>;
using Directionf = Direction<float>;

class Bounds2iIterator : public std::forward_iterator_tag
{
public:
    Bounds2iIterator() = delete;
    Bounds2iIterator(const Bounds2i& bounds, const Vec2i& p)
        : bounds_(bounds),
        point_(p)
    {}

    inline bool operator==(const Bounds2iIterator& rhs) const
    {
        return point_ == rhs.point_ && bounds_ == rhs.bounds_;
    }

    inline bool operator!=(const Bounds2iIterator& rhs) const
    {
        return point_ != rhs.point_ || bounds_ != rhs.bounds_;
    }

    Bounds2iIterator& operator++()
    {
        advance_();
        return *this;
    }

    Bounds2iIterator operator++(int)
    {
        Bounds2iIterator was = *this;
        advance_();
        return was;
    }

    Vec2i operator*() const
    {
        return point_;
    }

private:
    inline void advance_()
    {
        ++point_.x;
        if (point_.x == bounds_.max.x)
        {
            point_.x = bounds_.min.x;
            ++point_.y;
        }
    }

    const Bounds2i& bounds_;
    Vec2i point_;
};

inline Bounds2iIterator begin(const Bounds2i& b)
{
    return Bounds2iIterator(b, b.min);
}

inline Bounds2iIterator end(const Bounds2i& b)
{
    Vec2i vend(b.min.x, b.max.y);
    // just end the iterator right away if the bounds are degenerate
    if (b.min.x >= b.max.x || b.min.y >= b.max.y)
    {
        vend = b.min;
    }

    return Bounds2iIterator(b, vend);
}

struct Frustum
{
    Frustum(float fov, float aspectRatio, float n)
        : width( aspectRatio * 2.f * n * std::tan(0.5f * fov) ),
        height( 2.f * n * std::tan(0.5f * fov) ),
        near( n )
    {}

    Frustum(float fov, const Vec2i& resolution, float n)
        : width((float(resolution.x) / resolution.y) * 2.f * n * std::tan(0.5f * fov)),
        height(2.f * n * std::tan(0.5f * fov)),
        near(n)
    {}

    void setVerticalFov(float angle)
    {
        float ar = width / height;
        height = 2.f * near * std::tan(0.5f * angle);
        width = ar * height;
    }

    float width, height;
    float near;
};

}
