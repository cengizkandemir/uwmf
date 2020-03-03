// -*- mode: c++ -*-

#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "logger.h"
#include "../external/libpng-1.6.37/png.h"

namespace uwmf
{

class monochrome_png_image
{
public:
    monochrome_png_image()
        : image_{}
    {
    }

    monochrome_png_image(const std::string& file_name)
        : image_{}
    {
        if(!read(file_name)) {
            throw std::runtime_error("failed to read png file");
        }
    }

    bool read(const std::string& file_name)
    {
        image_.version = PNG_IMAGE_VERSION;
        if(!png_image_begin_read_from_file(&image_, file_name.c_str())) {
            LOGE() << "failed to begin reading png file";
            return false;
        }

        image_.format = PNG_FORMAT_GRAY;
        buffer_.resize(PNG_IMAGE_SIZE(image_));
        if(!png_image_finish_read(&image_, nullptr, buffer_.data(),
            0, nullptr)) {
            LOGE() << "failed to finish reading png file";
            return false;
        }

        return true;
    }

    bool write(const std::string& file_name)
    {
        if(!png_image_write_to_file(&image_, file_name.c_str(), 0, buffer_.data(),
            0, nullptr)) {
            LOGE() << "failed to write png file";
            return false;
        }

        return true;
    }

    std::size_t width() const
    {
        return image_.width;
    }

    std::size_t height() const
    {
        return image_.height;
    }

    const std::vector<char>& raw_data() const
    {
        return buffer_;
    }

private:
    png_image image_;
    std::vector<char> buffer_;
};

} // uwmf
