#pragma once
#include <cmath>
#include <cstdint>


template<typename T>
struct Vec2Base
{
    Vec2Base()
		: x{}
		, y{}
	{}

    Vec2Base(T x_, T y_)
		: x(x_)
		, y(y_)
	{}

    [[nodiscard]]
	float getLength2() const
	{
		return x * x + y * y;
	}

    [[nodiscard]]
	float getLength() const
	{
		return sqrt(getLength2());
	}

    Vec2Base operator/(float f) const
	{
		const float inv = 1.0f / f;
		return Vec2Base(x * inv, y * inv);
	}

    Vec2Base operator*(float f) const
	{
		return Vec2Base(x * f, y * f);
	}

    Vec2Base operator-(const Vec2Base& other) const
	{
		return Vec2Base(x - other.x, y - other.y);
	}

    Vec2Base operator-() const
	{
		return Vec2Base(-x, -y);
	}

	void operator+=(const Vec2Base& other)
	{
		x += other.x;
		y += other.y;
	}

	void operator+=(float f)
	{
		x += f;
		y += f;
	}

	void operator/=(float f)
	{
		x /= f;
		y /= f;
	}

    Vec2Base plus(const Vec2Base& other) const
	{
		return {x + other.x, y + other.y};
	}

    Vec2Base minus(const Vec2Base& other) const
	{
		return {x - other.x, y - other.y};
	}

	void operator-=(const Vec2Base& other)
	{
		x -= other.x;
		y -= other.y;
	}

	void rotate(const Vec2Base& origin, float angle)
	{
		const Vec2Base v = *this - origin;

		// This should be precomputed
		const float ca = cos(angle);
		const float sa = sin(angle);

		const float new_x = v.x * ca - v.y * sa;
		const float new_y = v.x * sa + v.y * ca;

		x = new_x + origin.x;
		y = new_y + origin.y;
	}

	void rotate(float angle)
	{
		// This should be precomputed
		const float ca = cos(angle);
		const float sa = sin(angle);

		const float new_x = x * ca - y * sa;
		const float new_y = x * sa + y * ca;

		x = new_x;
		y = new_y;
	}

    Vec2Base getNormal() const
	{
		return Vec2Base(-y, x);
	}

	float dot(const Vec2Base& other) const
	{
		return x * other.x + y * other.y;
	}

	float cross(const Vec2Base& other) const
	{
		return x * other.y - y * other.x;
	}

    Vec2Base getNormalized() const
	{
		return (*this) / getLength();
	}

	static Vec2Base rotate(Vec2Base v, const Vec2Base& origin, float angle)
	{
		v.rotate(origin, angle);
		return v;
	}

	T x, y;
};


using Vec2 = Vec2Base<float>;
using IVec2 = Vec2Base<int32_t>;
