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
    using value_iterator = typename std::vector<PixelValueType>::iterator;
    using const_value_iterator =
            typename std::vector<PixelValueType>::const_iterator;

public:
    using value_type = PixelValueType;

    template<typename IteratorType>
    class image_iterator;
    using iterator = image_iterator<value_iterator>;
    using const_iterator = image_iterator<const_value_iterator>;

    template<typename IteratorType>
    class image_iterator
    {
    public:
        struct pixel
        {
            std::size_t x;
            std::size_t y;
            IteratorType value;
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
            return {diff % stride_, diff / stride_, curr_};
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

    basic_image(const std::vector<PixelValueType>& buffer,
            std::size_t width, std::size_t height)
        : width_(width)
        , height_(height)
        , buffer_(buffer)
    {
    }

    basic_image(std::vector<PixelValueType>&& buffer,
            std::size_t width, std::size_t height)
        : width_(width)
        , height_(height)
        , buffer_(buffer)
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
        return iterator(std::begin(buffer_), width_);
    }

    auto begin() const
    {
        return const_iterator(std::begin(buffer_), width_);
    }

    auto end()
    {
        return iterator(std::end(buffer_), width_);
    }

    auto end() const
    {
        return const_iterator(std::end(buffer_), width_);
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

template<typename... ImageTypes>
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

    using iterator_tuple =
            std::tuple<decltype(std::declval<ImageTypes>().begin())...>;

    static constexpr auto begin_wrapper = [] (auto&& param)
    {
        return std::begin(std::forward<decltype(param)>(param));
    };

    static constexpr auto end_wrapper = [] (auto&& param)
    {
        return std::end(std::forward<decltype(param)>(param));
    };

public:
    template<typename IteratorTupleType>
    class zip_iterator
    {
    public:
        zip_iterator(IteratorTupleType iters)
            : iters_(iters)
        {
        }

        auto operator*() const
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
        IteratorTupleType iters_;
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
         return apply_impl<iterator_tuple>(begin_wrapper);
    }

    auto begin() const
    {
        return apply_impl<iterator_tuple>(begin_wrapper);
    }

    auto end()
    {
        return apply_impl<iterator_tuple>(end_wrapper);
    }

    auto end() const
    {
        return apply_impl<iterator_tuple>(end_wrapper);
    }

private:
    const std::tuple<ImageTypes...> images_;

    template<typename IteratorType, typename Accessor>
    auto apply_impl(Accessor accessor) const
    {
        return std::apply(
                [accessor] (auto&&... images)
                {
                    return zip_iterator<IteratorType>(
                            std::make_tuple(accessor(images)...));
                }, images_);
    }
};

template<typename... ImageTypes>
auto make_image_zip(ImageTypes&&... images)
{
    return image_zip_iterator<ImageTypes...>(
            std::forward<ImageTypes>(images)...);
}

} // uwmf
