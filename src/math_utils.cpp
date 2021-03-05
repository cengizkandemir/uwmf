#include "math_utils.h"

namespace uwmf
{

float mean(const monochrome_image& image)
{
    float sum = 0;

    for(const auto& [x, y, pixel]: image) {
        sum += *pixel;
    }

    return sum / (image.width() * image.height());
}

float variance(const monochrome_image& image, const float m)
{
    float sum = 0;

    for(const auto& [x, y, pixel]: image) {
        const auto val = *pixel - m;
        sum += (val * val);
    }

    return sum / (image.width() * image.height());
}

float variance(const monochrome_image& image)
{
    return variance(image, mean(image));
}

float covariance(const monochrome_image& image1, const float v1,
        const monochrome_image& image2, const float v2)
{
    float sum = 0;

    for(const auto& [px1, px2]: make_image_zip(image1, image2)) {
        sum += (*px1.value * *px2.value) - (v1 - v2);
    }

    return sum / (image1.width() * image1.height());
}

float covariance(const monochrome_image& image1, const monochrome_image& image2)
{
    const float v1 = variance(image1, mean(image1));
    const float v2 = variance(image2, mean(image2));
    return covariance(image1, v1, image2, v2);
}

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
