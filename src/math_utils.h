// -*- mode: c++ -*-

#pragma once

#include "image.h"

namespace uwmf
{

float mse(const monochrome_image& image1, const monochrome_image& image2);
float psnr(const monochrome_image& image1, const monochrome_image& image2);

} // uwmf
