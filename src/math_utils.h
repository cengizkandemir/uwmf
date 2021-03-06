// -*- mode: c++ -*-

#pragma once

#include "image.h"

namespace uwmf
{

double mean(const monochrome_image& image);
double variance(const monochrome_image& image, const double m);
double variance(const monochrome_image& image);
double covariance(const monochrome_image& image1,
        const monochrome_image& image2);
double covariance(const monochrome_image& image1, const double v1,
        const monochrome_image& image2, const double v2);
double se(const monochrome_image& image1, const monochrome_image& image2);
double mse(const monochrome_image& image1, const monochrome_image& image2);

} // uwmf
