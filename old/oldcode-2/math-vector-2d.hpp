#pragma once

#include <BWAPI.h>
#include <cmath>

struct Vector2D
{
    double x, y;
    Vector2D()
        : x(0.0), y(0.0)
    { }
    Vector2D(const BWAPI::Position& pos)
        : x(pos.x()), y(pos.y())
    { }
    Vector2D(const BWAPI::TilePosition& pos)
        : x(32*pos.x()), y(32*pos.y())
    { }
    Vector2D(double x_, double y_)
        : x(x_), y(y_)
    { }
    double lengthSqr() const
    {
        return x*x + y*y;
    }
    double length() const
    {
        return sqrt(lengthSqr());
    }
    Vector2D normalize() const
    {
        return (*this) / length();
    }
    Vector2D& operator += (const Vector2D& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    Vector2D& operator -= (const Vector2D& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    Vector2D& operator *= (double f)
    {
        x *= f;
        y *= f;
        return *this;
    }
    Vector2D& operator /= (double f)
    {
        x /= f;
        y /= f;
        return *this;
    }
    Vector2D operator + (const Vector2D& other) const
    {
        Vector2D result(*this);
        result += other;
        return result;
    }
    Vector2D operator - (const Vector2D& other) const
    {
        Vector2D result(*this);
        result -= other;
        return result;
    }
    Vector2D operator * (double f) const
    {
        Vector2D result(*this);
        result *= f;
        return result;
    }
    friend Vector2D operator * (double f, const Vector2D& vec)
    {
        return vec * f;
    }
    Vector2D operator / (double f) const
    {
        Vector2D result(*this);
        result *= f;
        return result;
    }
    double operator * (const Vector2D& other) const
    {
        return x * other.x + y * other.y;
    }
    bool isNull() const
    {
        return (std::fabs(x) < 1e-14) && (std::fabs(y) < 1e-14);
    }
    Vector2D rot() const
    {
        return Vector2D(-y, x);
    }
};
    