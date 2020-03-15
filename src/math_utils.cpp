#include "math_utils.h"

#include <cmath>
#include <limits>

namespace uwmf
{

float mse(const monochrome_image& image1, const monochrome_image& image2)
{
    int sum = 0;

    for(const auto& [px1, px2]: make_image_zip(image1, image2)) {
        const auto val = px1.value - px2.value;
        sum += val * val;
    }

    return static_cast<float>(sum / (image1.width() * image1.height()));
}

float psnr(const monochrome_image& image1, const monochrome_image& image2)
{
    constexpr auto max = std::numeric_limits<monochrome_image::value_type>::max();
    return 10 * std::log10((max * max) / mse(image1, image2));
}

} // uwmf
