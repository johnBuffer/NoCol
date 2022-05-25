#pragma once

template<typename T, uint64_t N>
struct Array
{
	T& operator[](const uint64_t index) {
		return data[index];
	}

	T operator[](const uint64_t index) const {
		return data[index];
	}

	T data[N];
};
