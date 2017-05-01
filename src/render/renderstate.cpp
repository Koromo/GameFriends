#include "renderstate.h"

GF_NAMESPACE_BEGIN

const DepthState DepthState::DEFAULT = {
    true,
    ComparisonFun::less
};

const RasterizerState RasterizerState::DEFAULT = {
    FillMode::solid,
    CullingFace::back,
    true
};

GF_NAMESPACE_END