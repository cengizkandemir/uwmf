// -*- mode: c++ -*-

#pragma once

#include <cstddef>
#include <iterator>
#include <ostream>
#include <utility>
#include <vector>

#include "utils.h"

namespace uwmf
{

template<typename PixelValueType>
class basic_image
{
    friend void swap(basic_image& first, basic_image& second) noexcept
    {
        using std::swap;
        swap(first.width_, second.width_);
        swap(first.height_, second.height_);
        swap(first.buffer_, second.buffer_);
    }

public:
    using val_type = PixelValueType;
    using iterator =
                typename std::vector<PixelValueType>::iterator;
    using const_iterator =
                typename std::vector<PixelValueType>::const_iterator;

    template<typename IteratorType>
    class image_iterator
    {
    public:
        struct pixel
        {
            std::size_t x;
            std::size_t y;
            PixelValueType value;
        };

        image_iterator(IteratorType iter, std::size_t stride)
            : start_(iter)
            , curr_(iter)
            , stride_(stride)
        {
        }

        image_iterator& operator++()
        {
            ++curr_;
            return *this;
        }

        pixel operator*() const
        {
            auto diff = curr_ - start_;
            return {diff % stride_, diff / stride_, *curr_};
        }

        bool operator!=(const image_iterator& other) const
        {
            return curr_ != other.curr_;
        }

    private:
        const IteratorType start_;
        IteratorType curr_;
        const std::size_t stride_;
    };

    basic_image()
        : width_(0)
        , height_(0)
    {
    }

    basic_image(std::size_t width, std::size_t height)
        : width_(width)
        , height_(height)
        , buffer_(width * height)
    {
    }

    basic_image(const basic_image& other)
        : width_(other.width_)
        , height_(other.height_)
        , buffer_(other.buffer_)
    {
    }

    basic_image(basic_image&& other) noexcept
        : width_(other.width_)
        , height_(other.height_)
        , buffer_(other.buffer_)
    {
        other.width_ = 0;
        other.height_ = 0;
    }

    basic_image& operator=(basic_image other)
    {
        swap(*this, other);
        return *this;
    }

    ~basic_image() = default;

    PixelValueType& operator()(std::size_t x, std::size_t y)
    {
        ASSERT(x + (width_ * y) < buffer_.size(), "indices out ouf bounds");
        return buffer_[x + (width_ * y)];
    }

    const PixelValueType& operator()(std::size_t x, std::size_t y) const
    {
        ASSERT(x + (width_ * y) < buffer_.size(), "indices out ouf bounds");
        return buffer_[x + (width_ * y)];
    }

    image_iterator<iterator> begin()
    {
        return image_iterator(buffer_.begin(), width_);
    }

    image_iterator<const_iterator> begin() const
    {
        return image_iterator(buffer_.begin(), width_);
    }

    image_iterator<iterator> end()
    {
        return image_iterator(buffer_.end(), width_);
    }

    image_iterator<const_iterator> end() const
    {
        return image_iterator(buffer_.end(), width_);
    }

    std::size_t width() const
    {
        return width_;
    }

    std::size_t height() const
    {
        return height_;
    }

private:
    std::size_t width_;
    std::size_t height_;
    std::vector<PixelValueType> buffer_;
};

template<typename PixelValueType>
std::ostream& operator<<(std::ostream& out,
        const basic_image<PixelValueType>& image)
{
    out << "\n";

    for(const auto& [x, y, intensity]: image) {
        out << "    ({" << x << ", " << y << "} -> "
                << static_cast<int>(intensity) << ")\n";
    }

    return out;
}

using monochrome_image = basic_image<char>;

template<typename PixelValueType>
class image_pair_view
{
public:
    class const_image_pair_iterator
    {
        using const_image_iterator =
                typename basic_image<PixelValueType>::template image_iterator<
                        typename basic_image<PixelValueType>::const_iterator>;
        using pixel_type =
                typename basic_image<PixelValueType>::template image_iterator<
                        typename basic_image<
                                PixelValueType>::const_iterator>::pixel;

    public:
        struct pixel_pair
        {
            pixel_type first;
            pixel_type second;
        };

        const_image_pair_iterator(const_image_iterator iter1,
                const_image_iterator iter2)
            : iter1_(iter1)
            , iter2_(iter2)
        {
        }

        pixel_pair operator*() const
        {
            return {*iter1_, *iter2_};
        }

        const_image_pair_iterator& operator++()
        {
            ++iter1_;
            ++iter2_;
            return *this;
        }

        bool operator!=(const const_image_pair_iterator& other) const
        {
            return iter1_ != other.iter1_ && iter2_ != other.iter2_;
        }

    private:
        const_image_iterator iter1_;
        const_image_iterator iter2_;
    };

    image_pair_view(const basic_image<PixelValueType>& image1,
            const basic_image<PixelValueType>& image2)
        : image1_(image1)
        , image2_(image2)
    {
        ASSERT(image1_.width() == image2_.width() &&
                image1_.height() == image2_.height(),
                "incompatible image dimensions");
    }

    const_image_pair_iterator begin() const
    {
        return const_image_pair_iterator(image1_.begin(), image2_.begin());
    }

    const_image_pair_iterator end() const
    {
        return const_image_pair_iterator(image1_.end(), image2_.end());
    }

private:
    const basic_image<PixelValueType>& image1_;
    const basic_image<PixelValueType>& image2_;
};

} // uwmf
