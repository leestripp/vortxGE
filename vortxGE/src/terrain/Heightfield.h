#pragma once

// ============================================================================
// Heightfield — owning float height grid + Span2D view.
// Plan.md §6 calls for std::mdspan (C++23); this toolchain's libstdc++ does
// not ship <mdspan> yet, so Span2D below mirrors the mdspan API surface we
// need and can be swapped out later.
// ============================================================================

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace terrain {

template <typename T>
class Span2D {
public:
    Span2D() = default;
    Span2D(T* data, std::size_t width, std::size_t height)
        : data_(data), w_(width), h_(height) {}

    T& operator()(std::size_t x, std::size_t y) const { return data_[y * w_ + x]; }
    std::size_t width() const { return w_; }
    std::size_t height() const { return h_; }
    std::size_t size() const { return w_ * h_; }
    T* data() const { return data_; }
    explicit operator bool() const { return data_ != nullptr; }

private:
    T* data_ = nullptr;
    std::size_t w_ = 0;
    std::size_t h_ = 0;
};

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

// Height grid with values normalized to [0,1]. u,v are normalized
// coordinates in [0,1] across the whole field.
class Heightfield {
public:
    Heightfield() = default;
    Heightfield(std::size_t w, std::size_t h, float init = 0.0f)
        : w_(w), h_(h), data_(w * h, init) {
        if (w == 0 || h == 0)
            throw std::invalid_argument("Heightfield: zero dimension");
    }

    std::size_t width() const { return w_; }
    std::size_t height() const { return h_; }

    float& operator()(std::size_t x, std::size_t y) { return data_[y * w_ + x]; }
    float operator()(std::size_t x, std::size_t y) const { return data_[y * w_ + x]; }

    float* data() { return data_.data(); }
    const float* data() const { return data_.data(); }

    Span2D<float> view() { return {data_.data(), w_, h_}; }
    Span2D<const float> view() const { return {data_.data(), w_, h_}; }

    // Bilinear sample in normalized [0,1] space (clamped at edges).
    float sampleBilinear(float u, float v) const {
        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);
        const float fx = u * static_cast<float>(w_ - 1);
        const float fy = v * static_cast<float>(h_ - 1);
        const std::size_t x0 = static_cast<std::size_t>(fx);
        const std::size_t y0 = static_cast<std::size_t>(fy);
        const std::size_t x1 = std::min(x0 + 1, w_ - 1);
        const std::size_t y1 = std::min(y0 + 1, h_ - 1);
        const float tx = fx - static_cast<float>(x0);
        const float ty = fy - static_cast<float>(y0);
        const float a = (*this)(x0, y0);
        const float b = (*this)(x1, y0);
        const float c = (*this)(x0, y1);
        const float d = (*this)(x1, y1);
        return std::lerp(std::lerp(a, b, tx), std::lerp(c, d, tx), ty);
    }

    // Central-difference gradient: d(normalizedHeight)/d(worldUnit).
    // Multiply by the world's height scale to get true dz/dx.
    Vec2 gradient(float u, float v, float worldSize) const {
        const float du = 1.0f / static_cast<float>(w_);
        const float dv = 1.0f / static_cast<float>(h_);
        const float cell = worldSize / static_cast<float>(w_);
        const float dzdx =
            (sampleBilinear(u + du, v) - sampleBilinear(u - du, v)) / (2.0f * cell);
        const float dzdy =
            (sampleBilinear(u, v + dv) - sampleBilinear(u, v - dv)) / (2.0f * cell);
        return {dzdx, dzdy};
    }

    float minValue() const { return *std::min_element(data_.begin(), data_.end()); }
    float maxValue() const { return *std::max_element(data_.begin(), data_.end()); }

    void normalize(float lo = 0.0f, float hi = 1.0f) {
        const float mn = minValue();
        const float mx = maxValue();
        const float range = mx - mn;
        if (range <= 0.0f)
            return;
        for (float& h : data_)
            h = lo + (h - mn) / range * (hi - lo);
    }

private:
    std::size_t w_ = 0;
    std::size_t h_ = 0;
    std::vector<float> data_;
};

} // namespace terrain
