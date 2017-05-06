#include "material.h"
#include "shademodel.h"
#include "texture.h"
#include "foundation/matrix44.h"
#include "foundation/color.h"
#include "foundation/vector4.h"

GF_NAMESPACE_BEGIN

Material::Material(const FilePath& path)
    : Resource(path)
    , params_()
    , shadeModelIn_()
    , shadeModel_()
{
}

void Material::setFloat(const std::string& name, float f)
{
    setNumeric(MatParamType::_float, name, &f, sizeof(f));
}

void Material::setFloat(const std::string& name, double f)
{
    setFloat(name, static_cast<float>(f));
}

void Material::setFloat4(const std::string& name, const Vector4& f4)
{
    setNumeric(MatParamType::_float4, name, &f4, sizeof(f4));
}

void Material::setFloat4(const std::string& name, const Color& f4)
{
    setNumeric(MatParamType::_float4, name, &f4, sizeof(f4));
}

void Material::setFloat4x4(const std::string& name, const Matrix44& f44)
{
    setNumeric(MatParamType::_float4x4, name, &f44, sizeof(f44));
}

void Material::setTex2D(const std::string& name, const ResourceInterface<MediaTexture>& texture)
{
    setTexture(MatParamType::_tex2d, name, texture);
}

void Material::directNumeric(ShaderType type, const std::string& name, const void* data, size_t size)
{
    shadeModelIn_->directNumeric(type, name, data, size);
}

OptimizedDrawCall Material::drawCallSource() const
{
    return shadeModelIn_->drawCallSource();
}

void Material::setNumeric(MatParamType type, const std::string& name, const void* data, size_t size)
{
    if (paramCheck(type, name))
    {
        shadeModelIn_->updateNumeric(name, data, size);
    }
}

void Material::setTexture(MatParamType type, const std::string& name, const ResourceInterface<MediaTexture>& texture)
{
    if (paramCheck(type, name) && texture.useable())
    {
        shadeModelIn_->updateTexture(name, texture->resource());
    }
}

bool Material::paramCheck(MatParamType type, const std::string& name)
{
    const auto it = params_.find(name);
    if (it == std::cend(params_))
    {
        return false;
    }

#ifdef GF_DEBUG
    check(it->second.type == type);
#endif

    return true;
}

GF_NAMESPACE_END