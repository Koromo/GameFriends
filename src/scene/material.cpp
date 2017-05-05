#include "material.h"
#include "texture.h"
#include "../foundation/matrix44.h"
#include "../foundation/color.h"
#include "../foundation/vector4.h"
#include <algorithm>
#include <cstring>

GF_NAMESPACE_BEGIN

const std::string AutoParameter::WORLD = "_World";
const std::string AutoParameter::VIEW = "_View";
const std::string AutoParameter::PROJ = "_Proj";
const std::string AutoParameter::TRANS = "_Trans";

bool isNumericParameter(ParameterType type)
{
    switch (type)
    {
    case ParameterType::_tex2d:
        return false;
    }
    return true;
}

size_t sizeofParameter(ParameterType type)
{
    switch (type)
    {
    case ParameterType::_float: return sizeof(float);
    case ParameterType::_float4: return sizeof(float) * 4;
    case ParameterType::_float4x4: return sizeof(float) * 16;
    }
    return 0;
}

size_t Pass::priority() const
{
    return priority_;
}

OptimizedDrawCall Pass::drawCallBase() const
{
    return drawCallBase_;
}

const ShaderParameters& Pass::shaderBindings() const
{
    return *params_;
}

void Pass::updateNumeric(const std::string& name, const void* data, size_t size)
{
    auto range = numericMappings_.equal_range(name);
    while (range.first != range.second)
    {
        const auto mapping = range.first->second;
        params_->updateConstant(mapping.toType, mapping.toName, data, size);
        ++range.first;
    }
}

void Pass::updateTexture(const std::string& name, PixelBuffer& tex)
{
    auto range = textureMappings_.equal_range(name);
    while (range.first != range.second)
    {
        const auto mapping = range.first->second;
        params_->updateShaderResource(mapping.toType, mapping.toName, tex);
        ++range.first;
    }
}

void Pass::setPriority(size_t i)
{
    priority_ = i;
}

void Pass::setDrawCallBase(const OptimizedDrawCall& base)
{
    drawCallBase_ = base;
    if (params_)
    {
        drawCallBase_.setShaderParameters(*params_);
    }
}

void Pass::setShaderProgram(const std::shared_ptr<const ShaderProgram>& program)
{
    program_ = program;
    params_ = program->createParameters();
    drawCallBase_.setShaderParameters(*params_);
}

void Pass::mapNumeric(const std::string& name, ShaderType toType, const std::string& toName)
{
    NumericMapping mapping;
    mapping.toType = toType;
    mapping.toName = toName;
    numericMappings_.emplace(name, mapping);
}

void Pass::mapTexture(const std::string& name, ShaderType toType, const std::string& toName)
{
    TextureMapping mapping;
    mapping.toType = toType;
    mapping.toName = toName;
    textureMappings_.emplace(name, mapping);
}

std::shared_ptr<Pass> Pass::duplicateWithNewBindings() const
{
    const auto newPass = std::make_shared<Pass>();
    newPass->priority_ = priority_;
    newPass->drawCallBase_ = drawCallBase_;
    newPass->program_ = program_;
    newPass->params_ = program_->createParameters();
    newPass->numericMappings_ = numericMappings_;
    newPass->textureMappings_ = textureMappings_;
    newPass->drawCallBase_.setShaderParameters(*newPass->params_);
    return newPass;
}

Material::Material(const std::string& path)
    : Resource(path)
    , doLoading_(true)
    , paramTable_()
    , passes_()
{
}

ResourceInterface<Material> Material::duplicate() const
{
    const auto newMaterial = std::make_shared<Material>(path());
    newMaterial->doLoading_ = false;
    newMaterial->load();
    newMaterial->doLoading_ = true;

    for (const auto& p : paramTable_)
    {
        newMaterial->paramTable_.emplace(p.first, Param{});
    }
    for (const auto& p : passes_)
    {
        newMaterial->passes_.insert(p->duplicateWithNewBindings());
    }

    for (const auto& p : paramTable_)
    {
        auto& copy = newMaterial->paramTable_[p.first];
        copy.type = p.second.type;

        if (isNumericParameter(copy.type))
        {
            const auto size = sizeofParameter(copy.type);
            copy.numeric.reset(new char[size]);
            newMaterial->setNumeric(p.second.type, p.first, p.second.numeric.get(), size);
        }
        else
        {
            newMaterial->setTexture(p.second.type, p.first, p.second.texture);
        }
    }

    return ResourceInterface<Material>::inplace(newMaterial);
}

void Material::setFloat(const std::string& name, float f)
{
    setNumeric(ParameterType::_float, name, &f, sizeof(f));
}

void Material::setFloat(const std::string& name, double f)
{
    setFloat(name, static_cast<float>(f));
}

void Material::setFloat4(const std::string& name, const Vector4& f4)
{
    setNumeric(ParameterType::_float4, name, &f4, sizeof(f4));
}

void Material::setFloat4(const std::string& name, const Color& f4)
{
    setNumeric(ParameterType::_float4, name, &f4, sizeof(f4));
}

void Material::setFloat4x4(const std::string& name, const Matrix44& f44)
{
    setNumeric(ParameterType::_float4x4, name, &f44, sizeof(f44));
}

void Material::setTex2D(const std::string& name, const ResourceInterface<MediaTexture>& texture)
{
    setTexture(ParameterType::_tex2d, name, texture);
}

size_t Material::numPasses() const
{
    return passes_.size();
}

Pass& Material::passNth(size_t n)
{
    check(n < passes_.size());
    return *passes_[n];
}

void Material::setNumeric(ParameterType type, const std::string& name, const void* data, size_t size)
{
    if (paramCheck(type, name))
    {
        for (const auto& pass : passes_)
        {
            pass->updateNumeric(name, data, size);
        }
        auto& param = paramTable_[name];
        std::memcpy(param.numeric.get(), data, std::min(size, sizeofParameter(type)));
    }
}

void Material::setTexture(ParameterType type, const std::string& name, const ResourceInterface<MediaTexture>& texture)
{
    if (paramCheck(type, name) && texture.useable())
    {
        for (const auto& pass : passes_)
        {
            pass->updateTexture(name, texture->resource());
        }
        paramTable_[name].texture = texture;
    }
}

bool Material::paramCheck(ParameterType type, const std::string& name)
{
    const auto it = paramTable_.find(name);
    if (it == std::cend(paramTable_))
    {
        return false;
    }

#ifdef GF_DEBUG
    check(it->second.type == type);
#endif

    return true;
}

MaterialLoadException::MaterialLoadException(const std::string& msg)
    : FileException(msg)
{
}

GF_NAMESPACE_END