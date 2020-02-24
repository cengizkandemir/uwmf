// -*- mode: c++ -*-

#pragma once

#include <cstddef>
#include <iterator>
#include <ostream>
#include <tuple>
#include <utility>
#include <vector>

#include "utils.h"

namespace uwmf
{

template<typename PixelValueType>
class basic_image
{
public:
    using value_type = PixelValueType;

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

        bool operator==(const image_iterator& other) const
        {
            return curr_ == other.curr_;
        }

        bool operator!=(const image_iterator& other) const
        {
            return !(curr_ == other.curr_);
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

    auto begin()
    {
        return image_iterator(std::begin(buffer_), width_);
    }

    auto begin() const
    {
        return image_iterator(std::begin(buffer_), width_);
    }

    auto end()
    {
        return image_iterator(std::end(buffer_), width_);
    }

    auto end() const
    {
        return image_iterator(std::end(buffer_), width_);
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

template<typename... ImageType>
class image_zip_iterator
{
    template <typename... Args>
    static bool all_equal(const Args&... args)
    {
        static_assert(sizeof...(Args) != 0);
        return [] (auto const& first, auto const&... rest)
               {
                   return ((first == rest) && ...);
               }(args...);
    }

public:
    template<typename... IteratorType>
    class zip_iterator
    {
        using pixel_tuple = std::tuple<typename IteratorType::pixel...>;
    public:

        zip_iterator(IteratorType&&... iters)
            : iters_(std::make_tuple(std::forward<IteratorType>(iters)...))
        {
        }

        pixel_tuple operator*() const
        {
            return std::apply(
                    [] (auto&&... iters)
                    {
                        return std::make_tuple(*iters...);
                    }, iters_);
        }

        zip_iterator& operator++()
        {
            std::apply(
                    [] (auto&&... iters)
                    {
                        (++iters, ...);
                    }, iters_);
            return *this;
        }

        bool operator==(const zip_iterator& other) const
        {
            return iters_ == other.iters_;
        }

        bool operator!=(const zip_iterator& other) const
        {
            return !(iters_ == other.iters_);
        }

    private:
        std::tuple<IteratorType...> iters_;
    };

    template<typename... Args>
    image_zip_iterator(Args&&... args)
        : images_(std::forward<Args>(args)...)
    {
        // TODO: static_assert that Args and ImageType are the same
        ASSERT(std::apply(
                [] (auto&&... images)
                {
                    return all_equal(images.width()...);
                }, images_), "incompatible image dimensions");
    }

    auto begin()
    {
        return std::apply(
                [] (auto&&... images)
                {
                    // TODO: forward the pack?
                    return zip_iterator(std::begin(images)...);
                }, images_);
    }

    auto begin() const
    {
        return std::apply(
                [] (auto&&... images)
                {
                    return zip_iterator(std::begin(images)...);
                }, images_);
    }

    auto end()
    {
        return std::apply(
                [] (auto&&... images)
                {
                    return zip_iterator(std::end(images)...);
                }, images_);
    }

    auto end() const
    {
        return std::apply(
                [] (auto&&... images)
                {
                    return zip_iterator(std::end(images)...);
                }, images_);
    }

private:
    const std::tuple<ImageType...> images_;
};

template<typename... ImageType>
inline auto make_image_zip(ImageType&&... images)
{
    return image_zip_iterator<ImageType...>(std::forward<ImageType>(images)...);
}

} // uwmf
