// -*- mode: c++ -*-

#pragma once

#include "image.h"
#include "utils.h"

#include <cmath>
#include <vector>

namespace uwmf
{

inline bool is_zero(double value)
{
    return std::fpclassify(value) == FP_ZERO;
}

double mean(const monochrome_image& image);
double variance(const monochrome_image& image, const double m);

inline double variance(const monochrome_image& image)
{
    return variance(image, mean(image));
}

double covariance(const monochrome_image& image1,
        const monochrome_image& image2);
double covariance(const monochrome_image& image1, const double v1,
        const monochrome_image& image2, const double v2);
double se(const monochrome_image& image1, const monochrome_image& image2);

inline double mse(const monochrome_image& image1,
        const monochrome_image& image2)
{
    return se(image1, image2) / (image1.width() * image1.height());
}

double minkowski_distance(const discrete_point2d& p1,
        const discrete_point2d& p2, const int p);

// calculates weights based on inverse Minkowski Distance
// p = 1 -> Manhattan Distance
// p = 2 -> Euclidean Distance
std::vector<double> gen_minkowski_weights(const int w, const int p);

} // uwmf
