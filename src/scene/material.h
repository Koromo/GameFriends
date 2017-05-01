#ifndef GAMEFRIENDS_MATERIAL_H
#define GAMEFRIENDS_MATERIAL_H

#include "../render/shaderprogram.h"
#include "../render/drawcall.h"
#include "../engine/resource.h"
#include "../foundation/prerequest.h"
#include "../foundation/sortedvector.h"
#include "../foundation/exception.h"
#include "../foundation/prerequest.h"
#include <string>
#include <memory>
#include <unordered_map>

GF_NAMESPACE_BEGIN

class PixelBuffer;
class MediaTexture;
class Vector4;
struct Color;
class Matrix44;

struct AutoParameter
{
    static const std::string WORLD; // float4x4 _World;
    static const std::string VIEW; // float4x4 _View;
    static const std::string PROJ; // float4x4 _Proj;

    static const std::string TRANS; // float4x4 _Trans
};

enum class ParameterType
{
    _float,
    _float4,
    _float4x4,
    _tex2d
};

bool isNumericParameter(ParameterType type);
size_t sizeofParameter(ParameterType type);

class Pass
{
private:
    size_t priority_;
    OptimizedDrawCall drawCallBase_;
    std::shared_ptr<const ShaderProgram> program_;
    std::shared_ptr<ShaderBindings> bindings_;

    struct NumericMapping
    {
        ShaderType toType;
        std::string toName;
    };
    struct TextureMapping
    {
        ShaderType toType;
        std::string toName;
    };
    std::unordered_multimap<std::string, NumericMapping> numericMappings_;
    std::unordered_multimap<std::string, TextureMapping> textureMappings_;

public:
    OptimizedDrawCall drawCallBase() const;
    const ShaderBindings& shaderBindings() const;

    void updateNumeric(const std::string& name, const void* data, size_t size);
    void updateTexture(const std::string& name, PixelBuffer& tex);

    size_t priority() const;

    void setPriority(size_t i);
    void setDrawCallBase(const OptimizedDrawCall& base);
    void setShaderProgram(const std::shared_ptr<const ShaderProgram>& program);
    void mapNumeric(const std::string& name, ShaderType toType, const std::string& toName);
    void mapTexture(const std::string& name, ShaderType toType, const std::string& toName);

    std::shared_ptr<Pass> duplicateWithNewBindings() const;
};

class Material : public Resource
{
private:
    bool doLoading_;

    struct Param
    {
        ParameterType type;
        ResourceInterface<MediaTexture> texture;
        std::shared_ptr<void> numeric;
    };
    std::unordered_map<std::string, Param> paramTable_;
    
    struct PassComp
    {
        bool operator()(const std::shared_ptr<Pass>& a, const std::shared_ptr<Pass>& b)
        {
            return a->priority() < b->priority();
        }
    };
    SortedVector<std::shared_ptr<Pass>, PassComp> passes_;

public:
    explicit Material(const std::string& path);

    ResourceInterface<Material> duplicate() const;

    void setFloat(const std::string& name, float f);
    void setFloat(const std::string& name, double f);
    void setFloat4(const std::string& name, const Vector4& f4);
    void setFloat4(const std::string& name, const Color& f4);
    void setFloat4x4(const std::string& name, const Matrix44& f44);
    void setTex2D(const std::string& name, const ResourceInterface<MediaTexture>& texture);

    size_t numPasses() const;
    Pass& passNth(size_t n);

private:
    void setNumeric(ParameterType type, const std::string& name, const void* data, size_t size);
    void setTexture(ParameterType type, const std::string& name, const ResourceInterface<MediaTexture>& texture);
    bool paramCheck(ParameterType type, const std::string& name);

    void loadImpl();
    void unloadImpl();
};

class MaterialLoadException : public FileException
{
public:
    explicit MaterialLoadException(const std::string& msg);
};

GF_NAMESPACE_END

#endif