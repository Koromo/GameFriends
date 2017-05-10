#ifndef GAMEFRIENDS_CODEC_H
#define GAMEFRIENDS_CODEC_H

#include "pixelformat.h"
#include "filesystem.h"
#include "foundation/prerequest.h"
#include <memory>
#include <string>
#include <vector>

GF_NAMESPACE_BEGIN

class Image
{
private:
    std::vector<Pixel_RGBA8_uint> pixels_;
    size_t width_;
    size_t height_;

public:
    Image(size_t w, size_t h);

    size_t width() const;
    size_t height() const;

    const Pixel_RGBA8_uint& at(size_t x, size_t y) const;
    Pixel_RGBA8_uint& at(size_t x, size_t y);

    const Pixel_RGBA8_uint* pixelArray() const;
};

std::shared_ptr<Image> decodeBmp(const EnginePath& path) noexcept(false);

GF_NAMESPACE_END

#endif