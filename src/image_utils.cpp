#include "image_utils.h"

#include "math_utils.h"

#include <cmath>
#include <limits>

namespace uwmf
{

double psnr(const monochrome_image& original, const monochrome_image& restored)
{
    constexpr auto max =
            std::numeric_limits<monochrome_image::value_type>::max();
    return 10 * std::log10((max * max) / mse(original, restored));
}

double ief(const monochrome_image& original, const monochrome_image& restored,
        const monochrome_image& noisy)
{
    return se(noisy, original) / se(restored, original);
}

double ssim(const monochrome_image& original, const monochrome_image& restored)
{
    // stabilization constants
    constexpr auto max =
            std::numeric_limits<monochrome_image::value_type>::max();
    constexpr double c1 = (0.01 * max) * (0.01 * max);
    constexpr double c2 = (0.03 * max) * (0.03 * max);

    const double mo = mean(original);
    const double mr = mean(restored);
    const double vo = variance(original, mo);
    const double vr = variance(restored, mr);
    const double covar = covariance(original, vo, restored, vr);

    return ((2 * mo * mr + c1) * (2 * covar + c2))
            / ((mo * mo + mr * mr + c1) * (vo * vo + vr * vr + c2));
}

monochrome_image fvin(monochrome_image original, const double density)
{
    random<double> noise(0, 1);
    random<int> type(0, 1);

    for(const auto& [x, y, pixel] : original) {
        if(noise.generate() < density) {
            *pixel = type.generate() == 0
                    ? std::numeric_limits<monochrome_image::value_type>::max()
                    : std::numeric_limits<monochrome_image::value_type>::min();
        }
    }

    return original;
}

} // uwmf
