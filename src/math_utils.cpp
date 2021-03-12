#include "math_utils.h"

namespace uwmf
{

double mean(const monochrome_image& image)
{
    double sum = 0;

    for(const auto& [x, y, pixel]: image) {
        sum += *pixel;
    }

    return sum / (image.width() * image.height());
}

double variance(const monochrome_image& image, const double m)
{
    double sum = 0;

    for(const auto& [x, y, pixel]: image) {
        const auto val = *pixel - m;
        sum += (val * val);
    }

    return sum / (image.width() * image.height());
}

double variance(const monochrome_image& image)
{
    return variance(image, mean(image));
}

double covariance(const monochrome_image& image1, const double v1,
        const monochrome_image& image2, const double v2)
{
    double sum = 0;

    for(const auto& [px1, px2]: make_image_zip(image1, image2)) {
        sum += (*px1.value * *px2.value) - (v1 - v2);
    }

    return sum / (image1.width() * image1.height());
}

double covariance(const monochrome_image& image1, const monochrome_image& image2)
{
    const double v1 = variance(image1, mean(image1));
    const double v2 = variance(image2, mean(image2));
    return covariance(image1, v1, image2, v2);
}

double se(const monochrome_image& image1, const monochrome_image& image2)
{
    double sum = 0;

    for(const auto& [px1, px2]: make_image_zip(image1, image2)) {
        const auto val = *px1.value - *px2.value;
        sum += val * val;
    }

    return sum;
}

double mse(const monochrome_image& image1, const monochrome_image& image2)
{
    return se(image1, image2) / (image1.width() * image1.height());
}

double minkowski_distance(const discrete_point2d& p1,
        const discrete_point2d& p2, const int p)
{
    double d1 = std::abs(p1.x - p2.x);
    double d2 = std::abs(p1.y - p2.y);
    d1 = std::pow(d1, p);
    d2 = std::pow(d2, p);
    return std::pow(d1 + d2, 1 / p);
}

std::vector<double> gen_minkowski_weights(const int w, const int p)
{
    const int edge_length = w * 2 + 1;
    const discrete_point2d center = {w, w};
    std::vector<double> weights(edge_length * edge_length);

    for(std::size_t i = 0; i < weights.size(); i++) {
        int x = i % edge_length;
        int y = i / edge_length;
        double distance = minkowski_distance(center, {x, y}, p);
        weights[i] = is_zero(distance) ? 0 : 1 / distance;
    }

    return weights;
}

} // uwmf
