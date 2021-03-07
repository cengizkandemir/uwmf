// -*- mode: c++ -*-

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace uwmf
{

struct monochrome_png_image
{
    std::size_t width;
    std::size_t height;
    std::vector<unsigned char> buffer;
};

std::optional<monochrome_png_image> read_png_image(
        const std::string& file_name);
bool write_png_image(const std::vector<unsigned char>& buffer,
        const std::size_t width, const std::size_t height,
        const std::string& file_name);

} // uwmf
