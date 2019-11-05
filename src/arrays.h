#pragma once

#include <vector>

using Bool = uint8_t;

template<typename T>
struct Array3D
{
public:
    Array3D() : width(0), height(0), depth(0) {}
    Array3D(size_t w, size_t h, size_t d, T value = {})
        : width(w), height(h), depth(d), data(w * h * d, value) {}

    const size_t index(size_t x, size_t y, size_t z) const {
        assert(x < width && "Error Array3D | x out of range");
        assert(y < height && "Error Array3D | y out of range");
        assert(z < depth && "Error Array3D | z out of range");
        // i*d2*d3 + j*d3 + k
        return x * height * depth + y * depth + z; // better cache hit ratio in our use case
    }

    inline T & operator()(size_t x, size_t y, size_t z) {
        return data[index(x,y,z)];
    }

    inline size_t size() const { return data.size(); }

private:
    size_t width, height, depth;
    std::vector<T> data;
};


template<typename T>
struct Array4D
{
public:
    Array4D() : width(0), height(0), depth(0), patterns(0) {}
    Array4D(size_t w, size_t h, size_t d, size_t t, T value = {})
        : width(w), height(h), depth(d), patterns(t), data(w * h * d * t, value) {}

    const size_t index(size_t x, size_t y, size_t z, size_t t) const {
        assert(x < width && "Error Array4D | x out of range");
        assert(y < height && "Error Array4D | y out of range");
        assert(z < depth && "Error Array4D | z out of range");
        assert(t < patterns && "Error Array4D | t out of range");
        // i*d2*d3*d4 + j*d3*d4 + k*d4 + t
        return x * height * depth * patterns + y * depth * patterns + z * patterns + t;
    }

    inline T & operator()(size_t x, size_t y, size_t z, size_t t) {
        return data[index(x,y,z,t)];
    }

    inline size_t size() const { return data.size(); }

private:
    size_t width, height, depth, patterns;
    std::vector<T> data;
};
