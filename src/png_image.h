// -*- mode: c++ -*-

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "logger.h"
#include "../external/libpng-1.6.37/png.h"

namespace uwmf
{

class gray_png_image
{
public:
    gray_png_image()
        : image_{}
    {
    }

    gray_png_image(std::string_view file_name)
        : image_{}
    {
        if(!read(file_name)) {
            throw std::runtime_error("failed to read png file");
        }
    }

    bool read(const std::string_view file_name)
    {
        image_.version = PNG_IMAGE_VERSION;
        if(!png_image_begin_read_from_file(&image_, file_name.data())) {
            LOGE() << "failed to begin reading png file";
            return false;
        }

        image_.format = PNG_FORMAT_GRAY;
        data_.resize(PNG_IMAGE_SIZE(image_));
        if(!png_image_finish_read(&image_, nullptr, data_.data(), 0, nullptr)) {
            LOGE() << "failed to finish reading png file";
            return false;
        }

        return true;
    }

    bool write(const std::string_view file_name)
    {
        if(!png_image_write_to_file(&image_, file_name.data(), 0, data_.data(), 0,
            nullptr)) {
            LOGE() << "failed to write png file";
            return false;
        }

        return true;
    }

    const std::vector<std::byte>& raw_data() const
    {
        return data_;
    }

private:
    png_image image_;
    std::vector<std::byte> data_;
};

} // uwmf
