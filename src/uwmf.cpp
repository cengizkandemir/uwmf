#include "uwmf.h"

#include "image.h"
#include "image_utils.h"
#include "logger.h"
#include "math_utils.h"
#include "utils.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{

using uwmf::discrete_point2d;
using size2d = uwmf::basic_point2d<std::size_t>;

struct convolution_indices
{
    discrete_point2d start;
    discrete_point2d end;
    std::size_t weight_start;
};

convolution_indices get_convolution_limits(const size2d& curr_coords,
        const discrete_point2d& image_size, const int w)
{
    int x = curr_coords.x;
    int y = curr_coords.y;
    const discrete_point2d start = {std::max(-w, -x), std::max(-w, -y)};
    const discrete_point2d end =
            {std::min(w, image_size.x - 1 - x),
            std::min(w, image_size.y - 1 - y)};
    std::size_t weight_index = (w + start.y) * (w * 2 + 1) + w + start.x;
    return {start, end, weight_index};
}

std::size_t leap_weight_index(const int w, const int startx, const int endx)
{
    return w * 2 + (startx - endx);
}


struct intermediates
{
    double S;
    double P;
    double Q;
    double R;
    double T;
};

/*
std::ostream& operator<<(std::ostream& out, const intermediates& interm)
{
    const auto [S, P, Q, R, T] = interm;
    out << "S: " << S << ", "
            << "P: " << P << ", "
            << "Q: " << Q << ", "
            << "R: " << R << ", "
            << "T: " << T;
    return out;
}
*/

template<typename Func>
void convolve(const size2d curr_coords,
        const discrete_point2d image_size,
        const uwmf::uwmf_parameters parameters,
        const Func func)
{
    const auto limits =
            get_convolution_limits(curr_coords, image_size, parameters.w);
    int weight_index = limits.weight_start;
    for(int yy = limits.start.y; yy <= limits.end.y; yy++) {
        for(int xx = limits.start.x; xx <= limits.end.x; xx++) {
            func(xx, yy, weight_index++);
        }
        weight_index += leap_weight_index(
                parameters.w, limits.start.x, limits.end.x);
    }
}

}

namespace uwmf
{

monochrome_image uwmf(const monochrome_image& corrupted_image,
        noise_detector detector, const uwmf_parameters parameters)
{
    const std::vector<double> org_weights =
            gen_minkowski_weights(parameters.w, parameters.p, parameters.k);
    std::vector<double> weights(org_weights.size());

    const discrete_point2d image_size =
            {static_cast<int>(corrupted_image.width()),
            static_cast<int>(corrupted_image.height())};
    monochrome_image restored_image(image_size.x, image_size.y);

    for(const auto& [x, y, pixel_it]: corrupted_image) {
        auto pixel = *pixel_it;

        if(!detector(pixel).first) {
            restored_image(x, y) = corrupted_image(x, y);
            continue;
        }

        weights = org_weights;

        bool all_corrupted = true;
        std::array<int, 2> corr_count{};
        auto interm = intermediates{};

        convolve({x, y}, image_size, parameters,
                [&, x = x, y = y]
                (const int xx, const int yy, const int weight_index)
                {
                    auto corr_result =
                            detector(corrupted_image(x + xx, y +  yy));
                    if(corr_result.first) {
                        corr_count[corr_result.second]++;
                    }
                    else {
                        all_corrupted = false;
                        const double weight = weights[weight_index];
                        interm.S += weight * yy * yy;
                        interm.P += weight * xx * xx;
                        interm.Q += weight * xx * yy;
                        interm.R += weight * xx;
                        interm.T += weight * yy;
                    }
                });

        if(all_corrupted) {
            constexpr auto min =
                    std::numeric_limits<monochrome_image::value_type>::min();
            constexpr auto max =
                    std::numeric_limits<monochrome_image::value_type>::max();
            constexpr auto salt = corruption::SALT;
            constexpr auto pepper = corruption::PEPPER;
            restored_image(x, y) = corr_count[salt] > corr_count[pepper]
                    ? min
                    : max;
            continue;
        }

        auto [S, P, Q, R, T] = interm;
        R = -R;
        T = -T;

        point2d gp;
        gp.y = ((P * T) - (Q * R)) / (-(Q * Q) + (P * S));
        gp.x = (R - (Q * gp.y)) / P;

        convolve({x, y}, image_size, parameters,
                [&, x = x, y = y]
                (const int xx, const int yy, const int weight_index)
                {
                    if(!detector(corrupted_image(x + xx, y + yy)).first) {
                        const double weight = weights[weight_index];
                        weights[weight_index] =
                                weight + (weight * (xx * gp.x + yy * gp.y));
                    }
                });

        double sumw = 0;
        double sumi = 0;
        double sumwo = 0;
        double sumio = 0;
        convolve({x, y}, image_size, parameters,
                [&, x = x, y = y]
                (const int xx, const int yy, const int weight_index)
                {
                    auto curr_pixel = corrupted_image(x + xx, y + yy);
                    if(!detector(curr_pixel).first) {
                        const double weight = weights[weight_index];
                        const double org_weight = org_weights[weight_index];
                        sumw += weight;
                        sumi += weight * curr_pixel;
                        sumwo += org_weight;
                        sumio += org_weight * curr_pixel;
                    }
                });

        if(sumw == 0) {
            restored_image(x, y) = sumio / sumwo;
        }
        else {
            restored_image(x, y) = sumi / sumw;
        }
    }

    return restored_image;
}


} // uwmf
