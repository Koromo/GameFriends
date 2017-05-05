#include "shademodel.h"
#include "foundation/exception.h"
#include <memory>
#include <algorithm>

GF_NAMESPACE_BEGIN

ShadeModelInput::ShadeModelInput(std::shared_ptr<ShaderParameters>&& programParams, const OptimizedDrawCall& drawCall,
    const std::unordered_multimap<std::string, detail::Mapping>& numericMappings,
    const std::unordered_multimap<std::string, detail::Mapping>& textureMappings_)
    : drawCall_(drawCall)
    , programParams_(std::move(programParams))
    , numericMappings_(numericMappings)
    , textureMappings_(textureMappings_)
{
    drawCall_.setShaderParameters(*programParams_);
}

OptimizedDrawCall ShadeModelInput::drawCallSource() const
{
    return drawCall_;
}

void ShadeModelInput::updateNumeric(const std::string& name, const void* data, size_t size)
{
    auto range = numericMappings_.equal_range(name);
    while (range.first != range.second)
    {
        const auto mapping = range.first->second;
        programParams_->updateConstant(mapping.toType, mapping.toName, data, std::min(size, mapping.maxSize));
        ++range.first;
    }
}

void ShadeModelInput::updateTexture(const std::string& name, PixelBuffer& texture)
{
    auto range = textureMappings_.equal_range(name);
    while (range.first != range.second)
    {
        const auto mapping = range.first->second;
        programParams_->updateShaderResource(mapping.toType, mapping.toName, texture);
        ++range.first;
    }
}

void ShadeModelInput::directNumeric(ShaderType type, const std::string& name, const void* data, size_t size)
{
    programParams_->updateConstant(type, name, data, size);
}

ShadeModel::ShadeModel(const std::string& path)
    : Resource(path)
    , drawCall_()
    , program_()
    , numericMappings_()
    , textureMappings_()
    , params_()
{
}

std::shared_ptr<ShadeModelInput> ShadeModel::createInput() const
{
    return std::make_shared<ShadeModelInput>(program_.createParameters(), drawCall_,
        numericMappings_, textureMappings_);
}

size_t ShadeModel::numParameters() const
{
    return params_.size();
}

void ShadeModel::parameter(size_t i, std::string& name, MatParamType& type) const
{
    name = params_[i].first;
    type = params_[i].second;
}

GF_NAMESPACE_END