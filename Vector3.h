#ifndef VECTOR3_H
#define VECTOR3_H

#include <iostream>
#include <cmath>

class Vector3
{
public:
    double x;
    double y;
    double z;

    Vector3() : x(0), y(0), z(0) {}

    Vector3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vector3 normalize() const
    {
        double length = sqrt(x * x + y * y + z * z);
        return Vector3(x / length, y / length, z / length);
    }

    Vector3 &operator+=(const Vector3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vector3 &operator*=(double t)
    {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vector3 operator-() const
    {
        return Vector3(-x, -y, -z);
    }


};

// use vector3 as Color
using Color3 = Vector3;

inline Vector3 operator*(double t, const Vector3 &v)
{
    return Vector3(t * v.x, t * v.y, t * v.z);
}

inline Vector3 operator*(const Vector3 &v, double t)
{
    return t * v;
}

inline double dot(const Vector3 &a, const Vector3 &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 operator+(const Vector3 &a, const Vector3 &b)
{
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vector3 operator-(const Vector3 &a, const Vector3 &b)
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3 cross(const Vector3& u, const Vector3& v) {
    return Vector3(u.y * v.z - u.z * v.y,
                   u.z * v.x - u.x * v.z,
                   u.x * v.y - u.y * v.x);
}

#endif // VECTOR3_H