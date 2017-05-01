#ifndef GAMEFRIENDS_PIXELFORMAT_H
#define GAMEFRIENDS_PIXELFORMAT_H

#include "../foundation/prerequest.h"

GF_NAMESPACE_BEGIN

enum class PixelFormat
{
    RGBA8_uint,
    RGBA8_unorm,

    R16,
    R16_float,
    RG16,
    RG16_float,

    R32,
    R32_float,
    RG32,
    RG32_float,

    RGB32_float,

    RGBA32_float,

    D16_unorm,
    D32_float
};

size_t sizeofPixelFormat(PixelFormat pf);

struct Pixel_RGBA8_uint
{
    uint8 R, G, B, A;
    static constexpr PixelFormat FORMAT = PixelFormat::RGBA8_uint;
};

GF_NAMESPACE_END

#endif
