#include "color.h"
#include "exception.h"

GF_NAMESPACE_BEGIN

const Color Color::RED = { 1, 0, 0, 1 };
const Color Color::GREEN = { 0, 1, 0, 1 };
const Color Color::BLUE = { 0, 0, 1, 1 };
const Color Color::WHITE = { 1, 1, 1, 1 };
const Color Color::BLACK = { 0, 0, 0, 1 };

Color::Color()
    : Color(Color::BLACK)
{
}

Color::Color(float r_, float g_, float b_, float a_)
    : r(r_)
    , g(g_)
    , b(b_)
    , a(a_)
{
}

const float& Color::operator [](size_t i) const
{
    return const_cast<Color&>(*this)[i];
}

float& Color::operator [](size_t i)
{
    switch (i)
    {
    case 0: return r;
    case 1: return g;
    case 2: return b;
    case 3: return a;
    default:
        check(false);
        return r;
    }
}

GF_NAMESPACE_END