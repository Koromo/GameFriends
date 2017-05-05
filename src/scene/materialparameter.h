#ifndef GAMEFRIENDS_MATERIALPARAMETER_H
#define GAMEFRIENDS_MATERIALPARAMETER_H

#include "foundation/prerequest.h"
#include <string>

GF_NAMESPACE_BEGIN

struct SystemMatParam
{
    static const std::string WORLD; // float4x4 _World;
    static const std::string VIEW; // float4x4 _View;
    static const std::string PROJ; // float4x4 _Proj;
};

enum class MatParamType
{
    _float,
    _float4,
    _float4x4,
    _tex2d
};

bool isNumeric(MatParamType type);
bool isFloating(MatParamType type);
size_t sizeofMatParam(MatParamType type);

GF_NAMESPACE_END

#endif