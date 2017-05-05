#ifndef GAMEFRIENDS_RENDERSTATE_H
#define GAMEFRIENDS_RENDERSTATE_H

#include "foundation/prerequest.h"

GF_NAMESPACE_BEGIN

enum class ComparisonFun
{
    never,
    less,
    equal,
    lessEqual,
    greater,
    notEqual,
    greaterEqual,
    always
};

enum class FillMode
{
    wireframe,
    solid,
};

enum class CullingFace
{
    none,
    front,
    back
};

struct DepthState
{
    bool depthTest;
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
