// -*- mode: c++ -*-

#pragma once

#include "image.h"
#include "image_utils.h"


namespace uwmf
{

struct uwmf_parameters
{
    int w;
    int p;
    int k;
};

monochrome_image uwmf(const monochrome_image& corrupted_image,
        noise_detector detector, const uwmf_parameters parameters);

monochrome_image UWMF(//graphics::basic_Canvas<float> &original,
		  const monochrome_image &image,
		  int wsize = 1, int p = 1, int k = 4, int offset = 0);

}
