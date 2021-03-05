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

} // uwmf
