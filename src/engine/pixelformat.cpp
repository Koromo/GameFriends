#include "pixelformat.h"
#include "../foundation/exception.h"

GF_NAMESPACE_BEGIN

size_t sizeofPixelFormat(PixelFormat pf)
{
    switch (pf)
    {
    case PixelFormat::RGBA8_uint:
    case PixelFormat::RGBA8_unorm:
        return 4;

    case PixelFormat::R16:
    case PixelFormat::R16_float:
        return 2;

    case PixelFormat::R32:
    case PixelFormat::R32_float:
        return 4;

    case PixelFormat::RG16:
    case PixelFormat::RG16_float:
        return 4;

    case PixelFormat::RG32:
    case PixelFormat::RG32_float:
        return 8;

    case PixelFormat::RGB32_float:
        return 12;

    case PixelFormat::RGBA32_float:
        return 16;

    case PixelFormat::D16_unorm:
        return 2;

    case PixelFormat::D32_float:
        return 4;

    default: check(false); return 0;
    }
}

GF_NAMESPACE_END