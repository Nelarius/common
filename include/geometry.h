#pragma once

#include "vector.h"
#include "matrix.h"
#include <cmath>
#include <iterator>
#include <limits>

namespace nlrs
{

// The plane normal is calculated from the input points in a counter clockwise sense,
// according to the right hand rule.

template<typename T>
class plane
{
public:
    plane() = delete;
    plane(const vector3<T>& p0, const vector3<T>& p1, const vector3<T>& p2)
        : edge1_{ p1 - p0 },
        edge2_{ p2 - p0 },
        point_{ p0 },
        normal_{ edge1_.cross(edge2_).normalized() }
    {}

    const vector3<T> point() const
    {
        return point_;
    }

    const vector3<T> normal() const
    {
        return normal_;
    }

private:
    vector3<T> edge1_;
    vector3<T> edge2_;
    vector3<T> point_;
    vector3<T> normal_;
};

template<typename T>
struct bounds2
{
    vector2<T> min{ std::numeric_limits<T>::max(), std::numeric_limits<T>::max() };
    vector2<T> max{ std::numeric_limits<T>::min(), std::numeric_limits<T>::min() };

    bounds2() = default;
    bounds2(const vector2<T>& mn, const vector2<T>& mx)
        : min(mn),
        max(mx)
    {}

    template<typename U>
    bounds2<U> cast() const
    {
        return bounds2<U>{ min.template cast<U>(), max.template cast<U>() };
    }

    bool operator==(const bounds2<T>& rhs) const
    {
        return min == rhs.min && max == rhs.max;
    }

    bool operator!=(const bounds2<T>& rhs) const
    {
        return min != rhs.min && max != rhs.max;
    }

    vector2<T> center() const
    {
        return T(0.5) * (min + max);
    }

    vector2<T> extent() const
    {
        return max - min;
    }

    bool contains(const vector2<T>& p)
    {
        if (p.x > min.x && p.x < max.x && p.y > min.y && p.y < max.y)
        {
            return true;
        }

        return false;
    }

    bounds2<T> shrink(T value) const
    {
        NLRS_ASSERT(max.x - min.x > value);
        NLRS_ASSERT(max.y - min.y > value);
        return bounds2<T>{ vector2<T>{ min.x + value, min.y + value }, vector2<T>{ max.x - value, max.y - value } };
    }

    bounds2<T> inflate(T value) const
    {
        return bounds2<T>{ vector2<T>{ min.x - value, min.y - value}, vector2<T>{ max.x + value, max.y + value } };
    }
};

template<typename T>
struct direction
{
    direction()
        : v(T(0.f), T(0.f), T(0.f), T(0.f))
    {}

    direction(T x, T y, T z)
        : v(x, y, z, 0.f)
    {}

    direction(const vector3<T>& v3)
        : v(v3.x, v3.y, v3.z, 0.f)
    {}

    direction(std::initializer_list<T> l)
        : v(0.f, 0.f, 0.f, 0.f)
    {
        NLRS_ASSERT(l.size() == 3u);
        u32 i = 0u;
        for (T t : l)
        {
            v.data[i] = t;
            ++i;
        }
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(v.data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(v.data)));
    }

    operator vector3<T>() const
    {
        return d;
    }

    operator vector4<T>() const
    {
        return v;
    }

    T dot(const direction& rhs) const
    {
        return d.dot(rhs.d);
    }

    direction cross(const direction& rhs) const
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

    T norm_squared() const
    {
        return d.normSquared();
    }

    direction operator+(const direction& rhs) const
    {
        return direction(d + rhs.d);
    }

    direction operator-(const direction& rhs) const
    {
        return direction(d - rhs.d);
    }

    direction operator-() const
    {
        return direction(-d);
    }

    direction operator*(T val) const
    {
        return direction(val * d);
    }

    direction operator/(T val) const
    {
        return direction(d / val);
    }

    direction& operator+=(const direction& rhs)
    {
        d += rhs.d;

        return *this;
    }

    direction& operator-=(const direction& rhs)
    {
        d -= rhs.d;

        return *this;
    }

    direction& operator*=(T val)
    {
        d *= val;

        return *this;
    }

    direction& operator/=(T val)
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
        vector3<T> d;
        vector4<T> v;
    };
};

template<typename T>
direction<T> operator*(T val, const direction<T>& rhs)
{
    return direction<T>(val * rhs.d);
}

template<typename T>
direction<T> operator*(const matrix4<T>& m, const direction<T>& d)
{
    return (direction<T>&)m * d.v;
}

template<typename T>
struct point
{
    point()
        : v(T(0.f), T(0.f), T(0.f), T(1.f))
    {}

    point(T x, T y, T z)
        : v(x, y, z, 1.f)
    {}

    point(const vector3<T>& v3)
        : v(v3.x, v3.y, v3.z, 1.f)
    {}

    point(std::initializer_list<T> l)
        : v(0.f, 0.f, 0.f, 1.f)
    {
        NLRS_ASSERT(l.size() == 3u);
        u32 i = 0u;
        for (T t : l)
        {
            v.data[i] = t;
            ++i;
        }
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_nans(v.data)));
        NLRS_ASSERT(!(detail::number<T, std::is_floating_point<T>::value>::has_infs(v.data)));
    }

    operator vector3<T>() const
    {
        return p;
    }

    operator vector4<T>() const
    {
        return v;
    }

    T norm() const
    {
        return p.norm();
    }

    T norm_squared() const
    {
        return p.normSquared();
    }

    point operator+(const point& rhs) const
    {
        return point(p + rhs.p);
    }

    point operator+(const direction<T>& rhs) const
    {
        return point(p + rhs.d);
    }

    point operator-(const point& rhs) const
    {
        return point(p - rhs.p);
    }

    point operator-(const direction<T>& rhs) const
    {
        return point(p - rhs.d);
    }

    point operator-() const
    {
        return point(-p);
    }

    point operator*(T val) const
    {
        return point(val * p);
    }

    point operator/(T val) const
    {
        return point(p / val);
    }

    point& operator+=(const point& rhs)
    {
        p += rhs.p;

        return *this;
    }

    point& operator+=(const direction<T>& rhs)
    {
        p += rhs.d;

        return *this;
    }

    point& operator-=(const point& rhs)
    {
        p -= rhs.p;

        return *this;
    }

    point& operator-=(const direction<T>& rhs)
    {
        p -= rhs.d;

        return *this;
    }

    point& operator*=(T val)
    {
        p *= val;

        return *this;
    }

    point& operator/=(T val)
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
        vector3<T> p;
        vector4<T> v;
    };
};

template<typename T>
point<T> operator*(T val, const point<T>& rhs)
{
    return point<T>(val * rhs.p);
}

template<typename T>
point<T> operator*(const matrix4<T>& m, const point<T>& p)
{

    return (point<T>&)m * p.v;
}

using planef = plane<float>;
using bounds2f = bounds2<float>;
using bounds2i = bounds2<i32>;
using pointf = point<float>;
using directionf = direction<float>;

class bounds2_iterator : public std::forward_iterator_tag
{
public:
    bounds2_iterator() = delete;
    bounds2_iterator(const bounds2i& bounds, const vec2i& p)
        : bounds_(bounds),
        point_(p)
    {}

    inline bool operator==(const bounds2_iterator& rhs) const
    {
        return point_ == rhs.point_ && bounds_ == rhs.bounds_;
    }

    inline bool operator!=(const bounds2_iterator& rhs) const
    {
        return point_ != rhs.point_ || bounds_ != rhs.bounds_;
    }

    bounds2_iterator& operator++()
    {
        advance();
        return *this;
    }

    bounds2_iterator operator++(int)
    {
        bounds2_iterator was = *this;
        advance();
        return was;
    }

    vec2i operator*() const
    {
        return point_;
    }

private:
    inline void advance()
    {
        ++point_.x;
        if (point_.x == bounds_.max.x)
        {
            point_.x = bounds_.min.x;
            ++point_.y;
        }
    }

    const bounds2i& bounds_;
    vec2i point_;
};

inline bounds2_iterator begin(const bounds2i& b)
{
    return bounds2_iterator(b, b.min);
}

inline bounds2_iterator end(const bounds2i& b)
{
    vec2i vend(b.min.x, b.max.y);
    // just end the iterator right away if the bounds are degenerate
    if (b.min.x >= b.max.x || b.min.y >= b.max.y)
    {
        vend = b.min;
    }

    return bounds2_iterator(b, vend);
}

struct frustum
{
    frustum(float fov, float aspect_ratio, float n)
        : width( aspect_ratio * 2.f * n * std::tan(0.5f * fov) ),
        height( 2.f * n * std::tan(0.5f * fov) ),
        near( n )
    {}

    frustum(float fov, const vec2i& resolution, float n)
        : width((float(resolution.x) / resolution.y) * 2.f * n * std::tan(0.5f * fov)),
        height(2.f * n * std::tan(0.5f * fov)),
        near(n)
    {}

    void set_vertical_fov(float angle)
    {
        float ar = width / height;
        height = 2.f * near * std::tan(0.5f * angle);
        width = ar * height;
    }

    float width, height;
    float near;
};

}
