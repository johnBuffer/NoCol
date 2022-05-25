#pragma once
#include <vector>
#include <array>
#include <SFML/System/Vector2.hpp>


template<typename T>
struct Grid
{
	struct HitPoint
	{
		T* cell;
		float dist;

		HitPoint()
			: cell(nullptr)
			, dist(0.0f)
		{}
	};

	int32_t width, height;
	std::vector<T> data;

	Grid()
		: width(0)
		, height(0)
	{
	}

	Grid(int32_t width_, int32_t height_)
		: width(width_)
		, height(height_)
	{
		data.resize(width * height);
	}

	int32_t mod(int32_t dividend, int32_t divisor) const
	{
		return (dividend%divisor + divisor) % divisor;
	}

	template<typename Vec2Type>
	bool checkCoords(const Vec2Type& v) const
	{
		return checkCoords(static_cast<int32_t>(v.x), static_cast<int32_t>(v.y));
	}

	bool checkCoords(int32_t x, int32_t y) const
	{
		return static_cast<int32_t>(x) > 0 && static_cast<int32_t>(x) < (width - 1) &&
               static_cast<int32_t>(y) > 0 && static_cast<int32_t>(y) < (height - 1);
	}

	const T& get(int32_t x, int32_t y) const
	{
		return data[y * width + x];
	}

	template<typename Vec2Type>
	T& get(const Vec2Type& v)
	{
		return get(v.x, v.y);
	}

	template<typename Vec2Type>
	const T& get(const Vec2Type& v) const
	{
		return get(static_cast<int32_t>(v.x), static_cast<int32_t>(v.y));
	}

	template<typename Vec2Type>
	const T& getWrap(Vec2Type v) const
	{
		return getWrap(v.x, v.y);
	}

	const T& getWrap(int32_t x, int32_t y) const
	{
		return get(mod(x, width), mod(y, height));
	}

	T& get(int32_t x, int32_t y)
	{
		return data[y * width + x];
	}

	template<typename Vec2Type>
	void set(const Vec2Type& v, const T& obj)
	{
		set(v.x, v.y, obj);
	}

	void set(int32_t x, int32_t y, const T& obj)
	{
		data[y * width + x] = obj;
	}
};
