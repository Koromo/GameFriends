#include "codec.h"
#include "../foundation/exception.h"
#include "../foundation/math.h"
#include <vector>
#include <cstdio>

GF_NAMESPACE_BEGIN

Image::Image(size_t w, size_t h)
    : pixels_(w * h)
    , width_(w)
    , height_(h)
{
}

size_t Image::width() const
{
    return width_;
}

size_t Image::height() const
{
    return height_;
}

const Pixel_RGBA8_uint& Image::at(size_t x, size_t y) const
{
    check(x < width_ && y < height_);
    return pixels_[width_ * y + x];
}

Pixel_RGBA8_uint& Image::at(size_t x, size_t y)
{
    return const_cast<Pixel_RGBA8_uint&>(static_cast<const Image&>(*this).at(x, y));
}

const Pixel_RGBA8_uint* Image::pixelArray() const
{
    return pixels_.data();
}

namespace
{
    const uint16 FILE_TYPE = 0x4d42; // Little endian
    const uint32 INFO_SIZE = 40; // Windows header
    const uint16 INFO_PLANES = 1;
    const uint16 INFO_BIT_COUNT = 24;
    const uint32 INFO_COMPRESSION = 0;

#pragma pack(2)
    struct BMPFileHeader
    {
        uint16 type;
        uint32 size;
        uint16 reserved1;
        uint16 reserved2;
        uint32 offset;
    };
#pragma pack()

    struct BMPInfoHeader
    {
        uint32 size;
        int32 width;
        int32 height;
        uint16 planes;
        uint16 bitCount;
        uint32 compression;
        uint32 sizeImage;
        int32 pelsPerMeterX;
        int32 pelsPerMeterY;
        uint32 colorUsed;
        uint32 colorImportant;
    };

    bool checkFormat(const BMPFileHeader& bf, const BMPInfoHeader& bi)
    {
        return
            bf.type == FILE_TYPE    && // Little endian only
            bi.size == INFO_SIZE    && // Windows bmp only
            bi.height > 0 &&           // Negative is deprecated format
            bi.planes == INFO_PLANES            && // We read an one image
            bi.bitCount == INFO_BIT_COUNT    && // 24 bit colors only
            bi.compression == INFO_COMPRESSION; // Non compression only
    }
}

std::shared_ptr<Image> decodeBmp(const std::string& path)
{
    auto file = enforce<FileException>(std::fopen(path.c_str(), "rb"), "Failed to open file (" + path + ").");
    GF_SCOPE_EXIT{ fclose(file); };

    BMPFileHeader bf;
    BMPInfoHeader bi;
    enforce<CodecException>(std::fread(&bf, sizeof(bf), 1, file) == 1, "Invalid .bmp file.");
    enforce<CodecException>(std::fread(&bi, sizeof(bi), 1, file) == 1, "Invalid .bmp file.");

    enforce<CodecException>(checkFormat(bf, bi), "Not supportted .bmp file format.");

    auto image = std::make_shared<Image>(bi.width, bi.height);

    const auto strideWidth = ceiling(bi.width * (bi.bitCount / 8), 4);
    std::vector<uint8> buffer(strideWidth);

    for (int32 y = bi.height - 1; y >= 0; --y)
    {
        enforce<CodecException>(std::fread(buffer.data(), strideWidth, 1, file) == 1, ".bmp read error.");
        auto it = std::cbegin(buffer);
        for (int32 x = 0; x < bi.width; ++x)
        {
            image->at(x, y).B = *it; ++it;
            image->at(x, y).G = *it; ++it;
            image->at(x, y).R = *it; ++it;
            image->at(x, y).R = 1;
        }
    }

    return image;
}

GF_NAMESPACE_END