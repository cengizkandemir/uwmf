// -*- mode: c++ -*-

#pragma once

#include "image.h"
#include "utils.h"

#include <limits>
#include <utility>

namespace uwmf
{

double psnr(const monochrome_image& original, const monochrome_image& restored);
double ief(const monochrome_image& original, const monochrome_image& restored,
        const monochrome_image& corrupted);
double ssim(const monochrome_image& original, const monochrome_image& restored);
monochrome_image fvin(monochrome_image original, const double density);


// Naive Noise Detection
// pixel with extreme values are considered corrupted

enum corruption
{
    SALT = 0,
    PEPPER = 1,
    NONE
};

inline std::pair<bool, corruption> naive_noise_detector(
        monochrome_image::value_type pixel)
{
    using value_type = monochrome_image::value_type;
    constexpr auto min = std::numeric_limits<value_type>::min();
    constexpr auto max = std::numeric_limits<value_type>::max();
    const bool salt = pixel == min;
    const bool pepper = pixel == max;
    const corruption type = salt
            ? corruption::SALT
            : pepper ? corruption::PEPPER : corruption::NONE;
    return {salt || pepper, type};
}

using noise_detector = decltype(naive_noise_detector);

} // uwmf
