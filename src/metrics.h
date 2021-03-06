// -*- mode: c++ -*-

#pragma once

#include "image.h"

namespace uwmf
{

double psnr(const monochrome_image& original, const monochrome_image& restored);
double ief(const monochrome_image& original, const monochrome_image& restored,
        const monochrome_image& noisy);
double ssim(const monochrome_image& original, const monochrome_image& restored);

} // uwmf
