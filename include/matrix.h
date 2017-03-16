#pragma once

#include "vector.h"
#include "quaternion.h"
#include <cmath>

namespace nlrs
{

template<typename T>
struct matrix2
{
    T data[4];

    matrix2()
        : data{ T(1.0), T(0.0), T(0.0), T(1.0) }
    {}

    matrix2(std::initializer_list<T> l)
    {
        int i = 0;
        for (T e : l)
        {
            data[i] = e;
            i++;
        }
    }

    matrix2(const vector2<T>& r1, const vector2<T>& r2)
        : data{ r1.x, r1.y, r2.x, r2.y }
    {}

    T trace() const
    {
        return data[0] + data[3];
    }

    vector2<T> operator*(const vector2<T>& v)
    {
        return vector2<T> {
            data[0] * v.x + data[1] * v.y,
                data[2] * v.y + data[3] * v.y
        };
    }

    matrix2<T> operator*(const matrix2<T>& m)
    {
        return matrix2<T> {
            data[0] * m.data[0] + data[1] * m.data[2],
                data[0] * m.data[1] + data[1] * m.data[3],
                data[2] * m.data[0] + data[3] * m.data[2],
                data[2] * m.data[1] + data[3] * m.data[3]
        };
    }

    matrix2<T> operator*(T val) const
    {
        return matrix2<T> {
            val*data[0], val*data[1],
                val*data[2], val*data[3]
        };
    }

    matrix2<T> operator-(const matrix2<T>& m) const
    {
        return matrix2<T> {
            data[0] - m.data[0], data[1] - m.data[1],
                data[2] - m.data[2], data[3] - m.data[3]
        };
    }

    matrix2<T> operator+(const matrix2<T>& m) const
    {
        return matrix2<T> {
            data[0] + m.data[0], data[1] + m.data[1],
                data[2] + m.data[2], data[3] + m.data[3]
        };
    }

    matrix2<T> transpose() const
    {
        return matrix2<T> {
            data[0], data[2],
                data[1], data[3]
        };
    }

    T determinant() const
    {
        return data[0] * data[3] - data[2] * data[1];
    }

    matrix2<T> inverse() const
    {
        // obtained using the Cayley-Hamilton method
        T factor = 1.0 / determinant();
        return factor * (trace()*matrix2<T>{} -*this);
    }
};

template<typename T>
matrix2<T> operator*(T val, const matrix2<T>&  m)
{
    return matrix2<T> {
        val*m.data[0], val*m.data[1],
            val*m.data[2], val*m.data[3]
    };
}

template<typename T>
struct matrix3
{
    T data[9];

    matrix3(std::initializer_list<T> l)
    {
        int i = 0;
        for (T e : l)
        {
            data[i] = e;
            i++;
        }
    }
    matrix3()
        : data{ T(1.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(1.0) }
    {}

    matrix3(const vector3<T>& r1, const vector3<T>& r2, const vector3<T>& r3)
        : data{ r1.x, r1.y, r1.z, r2.x, r2.y, r2.z, r3.x, r3.y, r3.z }
    {}

    T trace() const
    {
        return data[0] + data[4] + data[8];
    }

    vector3<T> operator*(const vector3<T>& v) const
    {
        return vector3<T> {
            data[0] * v.x + data[1] * v.y + data[2] * v.z,
                data[3] * v.x + data[4] * v.y + data[5] * v.z,
                data[6] * v.x + data[7] * v.y + data[8] * v.z
        };
    }

    matrix3<T> operator*(const matrix3<T>& m) const
    {
        return matrix3<T> {
            data[0] * m.data[0] + data[1] * m.data[3] + data[2] * m.data[6],
                data[0] * m.data[1] + data[1] * m.data[4] + data[2] * m.data[7],
                data[0] * m.data[2] + data[1] * m.data[5] + data[2] * m.data[8],
                data[3] * m.data[0] + data[4] * m.data[3] + data[5] * m.data[6],
                data[3] * m.data[1] + data[4] * m.data[4] + data[5] * m.data[7],
                data[3] * m.data[2] + data[4] * m.data[5] + data[5] * m.data[8],
                data[6] * m.data[0] + data[7] * m.data[3] + data[8] * m.data[6],
                data[6] * m.data[1] + data[7] * m.data[4] + data[8] * m.data[7],
                data[6] * m.data[2] + data[7] * m.data[5] + data[8] * m.data[8]
        };
    }

    matrix3<T> operator*(T val) const
    {
        return matrix3<T> {
            val*data[0], val*data[1], val*data[2],
                val*data[3], val*data[4], val*data[5],
                val*data[6], val*data[7], val*data[8]
        };
    }

    matrix3<T> operator+(const matrix3<T>& m) const
    {
        return matrix3<T> {
            data[0] + m.data[0], data[1] + m.data[1], data[2] + m.data[2],
                data[3] + m.data[3], data[4] + m.data[4], data[5] + m.data[5],
                data[6] + m.data[6], data[7] + m.data[7], data[8] + m.data[8]
        };
    }

    matrix3<T> operator-(const matrix3<T>& m) const
    {
        return matrix3<T> {
            data[0] - m.data[0], data[1] - m.data[1], data[2] - m.data[2],
                data[3] - m.data[3], data[4] - m.data[4], data[5] - m.data[5],
                data[6] - m.data[6], data[7] - m.data[7], data[8] - m.data[8]
        };
    }

    matrix3<T> transpose() const
    {
        return matrix3<T> {
            data[0], data[3], data[6],
                data[1], data[4], data[7],
                data[2], data[5], data[8]
        };
    }

    T determinant() const
    {
        return
            data[0] * matrix2<T>{ data[4], data[5], data[7], data[8] }.determinant()
            - data[1] * matrix2<T>{ data[3], data[5], data[6], data[8] }.determinant()
            + data[2] * matrix2<T>{ data[3], data[4], data[6], data[7] }.determinant();
    }

    matrix3<T> inverse() const
    {
        // obtained using the Cayley-Hamilton method
        T factor = 1.0 / determinant();
        const matrix3<T>& A = *this;
        matrix3<T> AA = A*A;
        T trA = A.trace();
        return factor * (matrix3<T>{}*0.5*(trA*trA - AA.trace()) - A*trA + AA);
    }
};

template<typename T>
matrix3<T> operator*(T val, const matrix3<T>& m)
{
    return matrix3<T> {
        val*m.data[0], val*m.data[1], val*m.data[2],
            val*m.data[3], val*m.data[4], val*m.data[5],
            val*m.data[6], val*m.data[7], val*m.data[8]
    };
}

template<typename T>
struct matrix4
{
    T data[16];

    matrix4(std::initializer_list<T> l)
    {
        int i = 0;
        for (T e : l)
        {
            data[i] = e;
            i++;
        }
    }

    matrix4()
        : data{ T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0), T(0.0), T(0.0), T(0.0), T(0.0), T(1.0) }
    {}

    matrix4(const vector4<T>& r1, const vector4<T>& r2, const vector4<T>& r3, const vector4<T>& r4)
        : data{ r1.x, r1.y, r1.z, r1.w, r2.x, r2.y, r2.z, r2.w, r3.x, r3.y, r3.z, r3.w, r4.x, r4.y, r4.z, r4.w }
    {}

    static matrix4<T> translation(const vector3<T>& v)
    {
        return matrix4<T>{
            T(1.0), T(0.0), T(0.0), v.x,
                T(0.0), T(1.0), T(0.0), v.y,
                T(0.0), T(0.0), T(1.0), v.z,
                T(0.0), T(0.0), T(0.0), T(1.0)
        };
    }

    static matrix4<T> rotation(const quaternion<T>& q)
    {
        T s = T(2.0) / q.norm();
        return matrix4<T> {
            1 - s*(q.v.y*q.v.y + q.v.z*q.v.z), s*(q.v.x*q.v.y - q.w*q.v.z), s*(q.v.x*q.v.z + q.w*q.v.y), T(0.0),
                s*(q.v.x*q.v.y + q.w*q.v.z), 1 - s*(q.v.x*q.v.x + q.v.z*q.v.z), s*(q.v.y*q.v.z - q.w*q.v.x), T(0.0),
                s*(q.v.x*q.v.z - q.w*q.v.y), s*(q.v.y*q.v.z + q.w*q.v.x), 1 - s*(q.v.x*q.v.x + q.v.y*q.v.y), T(0.0),
                T(0.0), T(0.0), T(0.0), T(1.0)
        };
    }

    static matrix4<T> scale(const vector3<T>& s)
    {
        return matrix4<T> {
            s.x, T(0.0), T(0.0), T(0.0),
                T(0.0), s.y, T(0.0), T(0.0),
                T(0.0), T(0.0), s.z, T(0.0),
                T(0.0), T(0.0), T(0.0), T(1.0)
        };
    }

    static matrix4<T> orthographic(T width, T height, T near, T far)
    {
        return matrix4<T>{
            T(2.0 / width), T(0.0), T(0.0), T(0.0),
                T(0.0), T(2.0) / height, T(0.0), T(0.0),
                T(0.0), T(0.0), T(2.0) / (near - far), (near + far) / (near - far),
                T(0.0), T(0.0), T(0.0), T(1.0)
        };
    }

    static matrix4<T> perspective(T vfov, T ar, T n, T f)
    {
        T h = T(2.0) * n * tan(T(0.5) * vfov);
        T w = ar * h;
        return matrix4<T>{
            T(2.0)*n / w, T(0.0), T(0.0), T(0.0),
                T(0.0), T(2.0)*n / h, T(0.0), T(0.0),
                T(0.0), T(0.0), -(f + n) / (f - n), -(T(2.0)*f*n) / (f - n),
                T(0.0), T(0.0), T(-1.0), T(0.0)
        };
    }

    T trace() const
    {
        return data[0] + data[5] + data[10] + data[15];
    }

    vector4<T> operator*(const vector4<T>& v) const
    {
        return vector4<T> {
            data[0] * v.x + data[1] * v.y + data[2] * v.z + data[3] * v.w,
                data[4] * v.x + data[5] * v.y + data[6] * v.z + data[7] * v.w,
                data[8] * v.x + data[9] * v.y + data[10] * v.z + data[11] * v.w,
                data[12] * v.x + data[13] * v.y + data[14] * v.z + data[15] * v.w,
        };
    }

    matrix4<T> operator*(const matrix4<T>& m) const
    {
        return matrix4<T> {
            data[0] * m.data[0] + data[1] * m.data[4] + data[2] * m.data[8] + data[3] * m.data[12],
                data[0] * m.data[1] + data[1] * m.data[5] + data[2] * m.data[9] + data[3] * m.data[13],
                data[0] * m.data[2] + data[1] * m.data[6] + data[2] * m.data[10] + data[3] * m.data[14],
                data[0] * m.data[3] + data[1] * m.data[7] + data[2] * m.data[11] + data[3] * m.data[15],
                data[4] * m.data[0] + data[5] * m.data[4] + data[6] * m.data[8] + data[7] * m.data[12],
                data[4] * m.data[1] + data[5] * m.data[5] + data[6] * m.data[9] + data[7] * m.data[13],
                data[4] * m.data[2] + data[5] * m.data[6] + data[6] * m.data[10] + data[7] * m.data[14],
                data[4] * m.data[3] + data[5] * m.data[7] + data[6] * m.data[11] + data[7] * m.data[15],
                data[8] * m.data[0] + data[9] * m.data[4] + data[10] * m.data[8] + data[11] * m.data[12],
                data[8] * m.data[1] + data[9] * m.data[5] + data[10] * m.data[9] + data[11] * m.data[13],
                data[8] * m.data[2] + data[9] * m.data[6] + data[10] * m.data[10] + data[11] * m.data[14],
                data[8] * m.data[3] + data[9] * m.data[7] + data[10] * m.data[11] + data[11] * m.data[15],
                data[12] * m.data[0] + data[13] * m.data[4] + data[14] * m.data[8] + data[15] * m.data[12],
                data[12] * m.data[1] + data[13] * m.data[5] + data[14] * m.data[9] + data[15] * m.data[13],
                data[12] * m.data[2] + data[13] * m.data[6] + data[14] * m.data[10] + data[15] * m.data[14],
                data[12] * m.data[3] + data[13] * m.data[7] + data[14] * m.data[11] + data[15] * m.data[15]
        };
    }

    matrix4<T> operator+(const matrix4<T>& m) const
    {
        return matrix4<T> {
            data[0] + m.data[0], data[1] + m.data[1], data[2] + m.data[2], data[3] + m.data[3],
                data[4] + m.data[4], data[5] + m.data[5], data[6] + m.data[6], data[7] + m.data[7],
                data[8] + m.data[8], data[9] + m.data[9], data[10] + m.data[10], data[11] + m.data[11],
                data[12] + m.data[12], data[13] + m.data[13], data[14] + m.data[14], data[15] + m.data[15]
        };
    }

    matrix4<T> operator-(const matrix4<T>& m) const
    {
        return matrix4<T> {
            data[0] - m.data[0], data[1] - m.data[1], data[2] - m.data[2], data[3] - m.data[3],
                data[4] - m.data[4], data[5] - m.data[5], data[6] - m.data[6], data[7] - m.data[7],
                data[8] - m.data[8], data[9] - m.data[9], data[10] - m.data[10], data[11] - m.data[11],
                data[12] - m.data[12], data[13] - m.data[13], data[14] - m.data[14], data[15] - m.data[15]
        };
    }

    matrix4<T> operator*(T val) const
    {
        return matrix4<T> {
            val*data[0], val*data[1], val*data[2], val*data[3],
                val*data[4], val*data[5], val*data[6], val*data[7],
                val*data[8], val*data[9], val*data[10], val*data[11],
                val*data[12], val*data[13], val*data[14], val*data[15]
        };
    }

    matrix4<T> transpose() const
    {
        return matrix4<T> {
            data[0], data[4], data[8], data[12],
                data[1], data[5], data[9], data[13],
                data[2], data[6], data[10], data[14],
                data[3], data[7], data[11], data[15]
        };
    }

    T determinant() const
    {
        return
            data[0] * matrix3<T>{ data[5], data[6], data[7], data[9], data[10], data[11], data[13], data[14], data[15] }.determinant()
            - data[1] * matrix3<T>{ data[4], data[6], data[7], data[8], data[10], data[11], data[12], data[14], data[15] }.determinant()
            + data[2] * matrix3<T>{ data[4], data[5], data[7], data[8], data[9], data[11], data[12], data[13], data[15] }.determinant()
            - data[3] * matrix3<T>{ data[4], data[5], data[6], data[8], data[9], data[10], data[12], data[13], data[14] }.determinant();
    }

    matrix4<T> inverse() const
    {
        // obtained using the Cayley-Hamilton method
        T factor = T(1.0) / determinant();
        const matrix4<T>& A = *this;
        matrix4<T> AA = A*A;
        matrix4<T> AAA = AA*A;
        T trA = trace();
        T trAA = AA.trace();
        T trAAA = AAA.trace();
        return factor * (matrix4<T>{}*T(0.1666666667*(trA*trA*trA - 3.0*trA*trAA + 2.0*trAAA)) - A*T(0.5)*(trA*trA - trAA) + AA*trA - AAA);
    }
};

template<typename T>
matrix4<T> operator*(T val, const matrix4<T>& m)
{
    return matrix4<T>{
        val*m.data[0], val*m.data[1], val*m.data[2], val*m.data[3],
            val*m.data[4], val*m.data[5], val*m.data[6], val*m.data[7],
            val*m.data[8], val*m.data[9], val*m.data[10], val*m.data[11],
            val*m.data[12], val*m.data[13], val*m.data[14], val*m.data[15]
    };
}

using mat2f = matrix2<float>;
using mat3f = matrix3<float>;
using mat4f = matrix4<float>;
using mat2d = matrix2<double>;
using mat3d = matrix3<double>;
using mat4d = matrix4<double>;

}
