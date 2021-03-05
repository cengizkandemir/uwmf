#include "math_utils.h"

#include <cmath>
#include <limits>

namespace uwmf
{

float psnr(const monochrome_image& original, const monochrome_image& restored)
{
    constexpr auto max =
            std::numeric_limits<monochrome_image::value_type>::max();
    return 10 * std::log10((max * max) / mse(original, restored));
}

float ief(const monochrome_image& original, const monochrome_image& restored,
        const monochrome_image& noisy)
{
    return se(noisy, original) / se(restored, original);
}

float ssim(const monochrome_image& original, const monochrome_image& restored)
{
    // stabilization constants
    constexpr auto max =
            std::numeric_limits<monochrome_image::value_type>::max();
    constexpr float c1 = (0.01 * max) * (0.01 * max);
    constexpr float c2 = (0.03 * max) * (0.03 * max);

    const float mo = mean(original);
    const float mr = mean(restored);
    const float vo = variance(original, mo);
    const float vr = variance(restored, mr);
    const float covar = covariance(original, vo, restored, vr);

    return ((2 * mo * mr + c1) * (2 * covar + c2))
            / ((mo * mo + mr * mr + c1) * (vo * vo + vr * vr + c2));
}

} // uwmf
