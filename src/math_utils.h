// -*- mode: c++ -*-

#pragma once

#include "image.h"

namespace uwmf
{

float mean(const monochrome_image& image);
float variance(const monochrome_image& image, const float m);
float variance(const monochrome_image& image);
float covariance(const monochrome_image& image1,
        const monochrome_image& image2);
float covariance(const monochrome_image& image1, const float v1,
        const monochrome_image& image2, const float v2);
float se(const monochrome_image& image1, const monochrome_image& image2);
float mse(const monochrome_image& image1, const monochrome_image& image2);

} // uwmf
