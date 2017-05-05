#include "materialparameter.h"

GF_NAMESPACE_BEGIN

const std::string SystemMatParam::WORLD = "_World";
const std::string SystemMatParam::VIEW = "_View";
const std::string SystemMatParam::PROJ = "_Proj";

bool isNumeric(MatParamType type)
{
    switch (type)
    {
    case MatParamType::_tex2d:
        return false;
    }
    return true;
}

bool isFloating(MatParamType type)
{
    switch (type)
    {
    case MatParamType::_float:
    case MatParamType::_float4:
    case MatParamType::_float4x4:
        return true;
    }
    return false;
}

size_t sizeofMatParam(MatParamType type)
{
    switch (type)
    {
    case MatParamType::_float: return sizeof(float);
    case MatParamType::_float4: return sizeof(float) * 4;
    case MatParamType::_float4x4: return sizeof(float) * 16;
    }
    return 0;

}
GF_NAMESPACE_END