#pragma once
#include "index_vector.hpp"
#include <sstream>
#include <SFML/Graphics.hpp>


template<typename U, typename T>
U to(const T& v)
{
    return static_cast<U>(v);
}


template<typename T>
using CIVector = civ::Vector<T>;


template<typename T>
T sign(T v)
{
    return v < 0.0f ? -1.0f : 1.0f;
}


template<typename T>
static std::string toString(T value)
{
    std::stringstream sx;
    sx << value;
    return sx.str();
}


template<typename T>
sf::Vector2f toVector2f(sf::Vector2<T> v)
{
    return {to<float>(v.x), to<float>(v.y)};
}


template<typename Vec3Type>
sf::Color toColor(Vec3Type v)
{
    return sf::Color{
        to<uint8_t>(v.x),
        to<uint8_t>(v.y),
        to<uint8_t>(v.z),
    };
}

template<template<typename> class ContainerType, typename T, typename TCallback>
void enumerate(ContainerType<T>& container, TCallback&& callback)
{
    uint64_t i(0);
    for (T& obj : container) {
        callback(i++, obj);
    }
}

template<typename TContainer, typename TPredicate>
void remove_if(TContainer& container, TPredicate&& pred)
{
    container.erase(std::remove_if(container.begin(), container.end(), pred), container.end());
}
