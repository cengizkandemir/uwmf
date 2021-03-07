#include "png_image.h"

#include "logger.h"
#include "../external/libpng-1.6.37/png.h"

namespace uwmf
{

std::optional<monochrome_png_image> read_png_image(
        const std::string& file_name)
{
    png_image image{};
    image.version = PNG_IMAGE_VERSION;

    if(!png_image_begin_read_from_file(&image, file_name.c_str())) {
        LOGE() << "failed to begin reading png file";
        return std::nullopt;
    }

    image.format = PNG_FORMAT_GRAY;
    std::vector<unsigned char> buffer(PNG_IMAGE_SIZE(image));
    if(!png_image_finish_read(&image, nullptr, buffer.data(),
                    0, nullptr)) {
        LOGE() << "failed to finish reading png file";
        return std::nullopt;
    }

    return monochrome_png_image{image.width, image.height, buffer};
}

bool write_png_image(const std::vector<unsigned char>& buffer,
        const std::size_t width, const std::size_t height,
        const std::string& file_name)
{
    png_image image{};
    image.format = PNG_FORMAT_GRAY;
    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;
    image.opaque = nullptr;
    image.flags = 0;
    image.colormap_entries = 256;
    return png_image_write_to_file(
            &image, file_name.c_str(), 0, buffer.data(), 0, nullptr);
}

} // uwmf
