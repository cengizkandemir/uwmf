#include "math_utils.h"

#include <cmath>
#include <limits>

namespace uwmf
{

float mse(const monochrome_image& image1, const monochrome_image& image2)
{
    int sum = 0;

    for(const auto& [px1, px2]: image_pair_view(image1, image2)) {
        sum += std::pow(px1.value - px2.value, 2);
    }

    return sum / (image1.width() * image1.height());
}

float psnr(const monochrome_image& image1, const monochrome_image& image2)
{
    return 10 * std::log10(std::pow(
            std::numeric_limits<monochrome_image::value_type>::max(), 2)
            / mse(image1, image2));
}

} // uwmf
