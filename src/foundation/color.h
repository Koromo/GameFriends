#ifndef GAMEFRIENDS_COLOR_H
#define GAMEFRIENDS_COLOR_H

#include "prerequest.h"

GF_NAMESPACE_BEGIN

struct Color
{
    float r, g, b, a;

    Color();
    Color(float r_, float g_, float b_, float a_);
    Color(const Color&) = default;

    Color& operator =(const Color&) = default;

    const float& operator [](size_t i) const;
    float& operator [](size_t i);

    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color WHITE;
    static const Color BLACK;
};

GF_NAMESPACE_END

#endif