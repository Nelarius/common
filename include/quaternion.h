#pragma once

#include "vector.h"
#include "nlrs_assert.h"
#include <cmath>

namespace nlrs
{

template<typename T>
class quaternion
{

public:
    quaternion() = default;
    quaternion(const quaternion&) = default;
    quaternion(quaternion&&) = default;
    quaternion& operator=(const quaternion&) = default;
    quaternion& operator=(quaternion&&) = default;

    static quaternion<T> identity()
    {
        return quaternion<T>{ 0.0, 0.0f, 0.0f, 1.0f };
    }

    quaternion(const vector3<T>& i, T r)
        : v(i),
        w(r)
    {}

    quaternion(const vector4<T>& vec)
        : v{ vec.x, vec.y, vec.z },
        w{ vec.w }
    {}

    quaternion(T x, T y, T z, T w)
        : v(x, y, z),
        w(w)
    {}

    // get the rotation between two normalized axes
    static quaternion<T> rotation_between_axes(vector3<T> s, vector3<T> t)
    {
        NLRS_ASSERT(s.norm() - T(1.0) < T(0.00001));
        NLRS_ASSERT(t.norm() - T(1.0) < T(0.00001));
        T e = s.dot(t);
        T div = T(1.0) / std::sqrt(T(2.0) * (T(1.0) + e));
        return quaternion<T> {
            s.cross(t).normalized() * div, T(2.0) * div
        };
    }

    static quaternion<T> from_axis_angle(vector3<T> axis, T angle)
    {
        axis.normalize();
        axis = axis * std::sin(T(0.5) * angle);
        return quaternion<T>{ axis, std::cos(T(0.5) * angle) };
    }

    template<typename D>
    quaternion<D> cast() const
    {
        return quaternion<D>{ D(v.x), D(v.y), D(v.z), D(w) };
    }

    operator vector4<T>() const
    {
        return vector4<T> {v.x, v.y, v.z, w};
    }

    quaternion<T> conjugate() const
    {
        return quaternion<T>{ -v.x, -v.y, -v.z, w };
    }

    quaternion<T> inverse() const
    {
        T factor = 1.0f / normSquared();
        return conjugate() * factor;
    }

    T norm() const
    {
        return std::sqrt(v.normSquared() + w*w);
    }

    T norm_squared() const
    {
        return v.normSquared() + w*w;
    }

    quaternion<T> normalized() const
    {
        T n = T(1.0) / norm();
        return quaternion<T>{
            v.x*n, v.y*n, v.z*n, w*n
        };
    }

    void normalize()
    {
        T n = T(1.0) / norm();
        v.x *= n;
        v.y *= n;
        v.z *= n;
        w *= n;
    }

    quaternion<T> operator*(const quaternion& rhs) const
    {
        return quaternion<T> {
            v.cross(rhs.v) + rhs.w*v + w*rhs.v,
                w*rhs.w - v.dot(rhs.v)
        };
    }

    quaternion<T> operator*(T rhs) const
    {
        return quaternion<T> {
            rhs*v.x,
                rhs*v.y,
                rhs*v.z,
                rhs*w
        };
    }

    quaternion<T> multiply(const quaternion& rhs) const
    {
        return quaternion<T> {
            v.cross(rhs.v) + rhs.w*v + w*rhs.v,
                w*rhs.w - v.dot(rhs.v)
        };
    }

    vector4<T> rotate(const vector4<T>& rhs) const
    {
        // this could use optimization to get rid of the intermediates...
        quaternion<T> intermediate = this->multiply(*reinterpret_cast<const quaternion<T>*>(&rhs));
        return intermediate.multiply(this->conjugate());
    }

    vector3<T> axis() const
    {
        T angle = this->angle();
        return v * (T(1.0) / sin(angle / T(2.0)));
    }

    T angle() const
    {
        return T(2.0 * acos(w));
    }

    vector3<T> xaxis() const
    {
        T n = T(2.f) / norm();
        return vector3<T> {
            1.f - n*(v.y*v.y + v.z*v.z),
                n*(v.x*v.y + w*v.z),
                n*(v.x*v.z - w*v.y)
        };
    }

    vector3<T> yaxis() const
    {
        T n = T(2.f) / norm();
        return vector3<T> {
            n*(v.x*v.y - w*v.z),
                1.f - n*(v.x*v.x + v.z*v.z),
                n*(v.y*v.z + w*v.x)
        };
    }

    vector3<T> zaxis() const
    {
        T n = T(2.f) / norm();
        return vector3<T>{
            n*(v.x*v.z + w*v.y),
                n*(v.y*v.z - w*v.x),
                1.f - n*(v.x*v.x + v.y*v.y)
        };
    }

    vector3<T>  v{};    // the imaginary part
    T           w{ T(1.0) }; // the real part
};

template<typename T>
quaternion<T> operator*(T lhs, const quaternion<T>& rhs)
{
    return quaternion<T>{ lhs*rhs.v.x, lhs*rhs.v.y, lhs*rhs.v.z, lhs*rhs.w };
}

using quatf = quaternion<float>;
using quatd = quaternion<double>;

}
