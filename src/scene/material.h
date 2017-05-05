#ifndef GAMEFRIENDS_MATERIAL_H
#define GAMEFRIENDS_MATERIAL_H

#include "materialparameter.h"
#include "../render/shaderprogram.h"
#include "../render/drawcall.h"
#include "../engine/resource.h"
#include "foundation/prerequest.h"

GF_NAMESPACE_BEGIN

class MediaTexture;
class ShadeModel;
class ShadeModelInput;
class Vector4;
struct Color;
class Matrix44;

class Material : public Resource
{
private:
    struct ParamHolder
    {
        MatParamType type;
        ResourceInterface<MediaTexture> texture;
        std::shared_ptr<void> numeric;
    };
    std::unordered_map<std::string, ParamHolder> params_;
    std::shared_ptr<ShadeModelInput> shadeModelIn_;
    ResourceInterface<const ShadeModel> shadeModel_;

public:
    explicit Material(const std::string& path);

    void setFloat(const std::string& name, float f);
    void setFloat(const std::string& name, double f);
    void setFloat4(const std::string& name, const Vector4& f4);
    void setFloat4(const std::string& name, const Color& f4);
    void setFloat4x4(const std::string& name, const Matrix44& f44);
    void setTex2D(const std::string& name, const ResourceInterface<MediaTexture>& texture);

    void directNumeric(ShaderType type, const std::string& name, const void* data, size_t size);
    OptimizedDrawCall drawCallSource() const;

private:
    void setNumeric(MatParamType type, const std::string& name, const void* data, size_t size);
    void setTexture(MatParamType type, const std::string& name, const ResourceInterface<MediaTexture>& texture);
    bool paramCheck(MatParamType type, const std::string& name);

    void loadImpl();
    void unloadImpl();
};

GF_NAMESPACE_END

#endif