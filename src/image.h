// -*- mode: c++ -*-

#pragma once

#include <cstddef>
#include <iterator>
#include <ostream>
#include <utility>
#include <vector>

#include "utils.h"

template<typename PixelValueType>
class image
{
    friend void swap(image& first, image& second) noexcept
    {
        using std::swap;
        swap(first.width_, second.width_);
        swap(first.height_, second.height_);
        swap(first.buffer_, second.buffer_);
    }

    friend std::ostream& operator<<(std::ostream& out, const image& img)
    {
        out << "\n";

        for(const auto& [x, y, intensity]: img) {
            out << "    ({" << x << ", " << y << "} -> "
                    << static_cast<int>(intensity) << ")\n";
        }

        return out;
    }

public:
    class const_image_iterator
    {
        using const_iterator =
                typename std::vector<PixelValueType>::const_iterator;

    public:
        struct pixel
        {
            std::size_t x;
            std::size_t y;
            PixelValueType value;
        };

        const_image_iterator(const_iterator iter, std::size_t stride)
            : start_(iter)
            , curr_(iter)
            , stride_(stride)
        {
        }

        const_image_iterator& operator++()
        {
            ++curr_;
            return *this;
        }

        pixel operator*() const
        {
            auto diff = curr_ - start_;
            return {diff % stride_, diff / stride_, *curr_};
        }

        bool operator!=(const const_image_iterator& other) const
        {
            return curr_ != other.curr_;
        }

    private:
        const const_iterator start_;
        const_iterator curr_;
        const std::size_t stride_;
    };

    image()
        : width_(0)
        , height_(0)
    {
    }

    image(std::size_t width, std::size_t height)
        : width_(width)
        , height_(height)
        , buffer_(width * height)
    {
    }

    image(const image& other)
        : width_(other.width_)
        , height_(other.height_)
        , buffer_(other.buffer_)
    {
    }

    image(image&& other) noexcept
        : width_(other.width_)
        , height_(other.height_)
        , buffer_(other.buffer_)
    {
        other.width_ = 0;
        other.height_ = 0;
    }

    image& operator=(image other)
    {
        swap(*this, other);
        return *this;
    }

    ~image() = default;

    PixelValueType& operator()(std::size_t x, std::size_t y) noexcept
    {
        ASSERT(x + (width_ * y) < buffer_.size(), "indices out ouf bounds");
        return buffer_[x + (width_ * y)];
    }

    const PixelValueType& operator()(std::size_t x, std::size_t y) const noexcept
    {
        ASSERT(x + (width_ * y) < buffer_.size(), "indices out ouf bounds");
        return buffer_[x + (width_ * y)];
    }

    const_image_iterator begin() const
    {
        return const_image_iterator(buffer_.begin(), width_);
    }

    const_image_iterator end() const
    {
        return const_image_iterator(buffer_.end(), width_);
    }


private:
    std::size_t width_;
    std::size_t height_;
    std::vector<PixelValueType> buffer_;
};
