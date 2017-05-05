#ifndef GAMEFRIENDS_RENDERSTATE_H
#define GAMEFRIENDS_RENDERSTATE_H

#include "foundation/prerequest.h"

GF_NAMESPACE_BEGIN

enum class ComparisonFun
{
    never = 0,
    less = 1,
    equal = 2,
    lessEqual = 3,
    greater = 4,
    notEqual = 5,
    greaterEqual = 6,
    always = 7
};

enum class FillMode
{
    wireframe = 0,
    solid = 1,
};

enum class CullingFace
{
    none = 0,
    front = 1,
    back = 2
};

struct DepthState
{
    bool depthEnable;
    ComparisonFun depthFun;
    static const DepthState DEFAULT;
};

struct RasterizerState
{
    FillMode fillMode;
    CullingFace cullFace;
    bool depthClip;
    static const RasterizerState DEFAULT;
};

struct Viewport
{
    float left;
    float top;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

GF_NAMESPACE_END

#endif
