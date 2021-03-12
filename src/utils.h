// -*- mode: c++ -*-

#pragma once

#include <cassert>
#include <random>
#include <type_traits>

#define ASSERT(predicate, message) assert(predicate && message)

#define DELETE_COPY_AND_ASSIGN(class_name)              \
    class_name(const class_name&) = delete;             \
    class_name& operator=(const class_name&) = delete

namespace uwmf
{

template<typename ValueType>
struct basic_point2d
{
    ValueType x;
    ValueType y;
};

using point2d = basic_point2d<double>;
using discrete_point2d = basic_point2d<int>;

template<typename ValueType>
class random
{
    template<typename... Args>
    static constexpr bool match_any()
    {
        return (std::is_same_v<ValueType, Args> || ...);
    }

    static constexpr bool is_type_supported()
    {
        if constexpr(match_any<bool, char, char16_t, char32_t, wchar_t>()) {
            return false;
        }

        return std::is_integral_v<ValueType> ||
                std::is_floating_point_v<ValueType>;
    }

public:
    random(const ValueType min, const ValueType max)
        : dist_(min, max)
    {
        static_assert(is_type_supported());
    }

    ValueType generate()
    {
        return dist_(gen_);
    }

private:
    std::random_device dev_;
    std::mt19937 gen_;
    std::conditional_t<std::is_floating_point_v<ValueType>,
            std::uniform_real_distribution<ValueType>,
            std::uniform_int_distribution<ValueType>> dist_;
};

} // uwmf
