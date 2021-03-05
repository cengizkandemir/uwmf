#include "math_utils.h"

namespace uwmf
{

float se(const monochrome_image& image1, const monochrome_image& image2)
{
    float sum = 0;

    for(const auto& [px1, px2]: make_image_zip(image1, image2)) {
        const auto val = px1.value - px2.value;
        sum += val * val;
    }

    return sum;
}

float mse(const monochrome_image& image1, const monochrome_image& image2)
{
    return se(image1, image2) / (image1.width() * image1.height());
}

} // uwmf
