// -*- mode: c++ -*-

#pragma once

#include "image.h"

namespace uwmf
{

float psnr(const monochrome_image& original, const monochrome_image& restored);
float ief(const monochrome_image& original, const monochrome_image& restored,
        const monochrome_image& noisy);

} // uwmf
