#ifndef GAMEFRIENDS_SHADEMODEL_H
#define GAMEFRIENDS_SHADEMODEL_H

#include "materialparameter.h"
#include "../render/shaderprogram.h"
#include "../render/drawcall.h"
#include "../engine/resource.h"
#include "../engine/filesystem.h"
#include "foundation/prerequest.h"
#include <memory>
#include <vector>

GF_NAMESPACE_BEGIN

class PixelBuffer;

namespace detail
{
    struct Mapping
    {
        size_t maxSize;
        ShaderType toType;
        std::string toName;
    };
}

class ShadeModelInput
{
private:
    OptimizedDrawCall drawCall_;
    std::shared_ptr<ShaderParameters> programParams_;

    const std::unordered_multimap<std::string, detail::Mapping>& numericMappings_;
    const std::unordered_multimap<std::string, detail::Mapping>& textureMappings_;

public:
    ShadeModelInput(std::shared_ptr<ShaderParameters>&& programParams, const OptimizedDrawCall& drawCallBase,
        const std::unordered_multimap<std::string, detail::Mapping>& numericMappings,
        const std::unordered_multimap<std::string, detail::Mapping>& textureMappings_);

    OptimizedDrawCall drawCallSource() const;

    void updateNumeric(const std::string& name, const void* data, size_t size);
    void updateTexture(const std::string& name, PixelBuffer& texture);

    void directNumeric(ShaderType type, const std::string& name, const void* data, size_t size);
};

class ShadeModel : public Resource
{
private:
    OptimizedDrawCall drawCall_;
    ShaderProgram program_;
    std::unordered_multimap<std::string, detail::Mapping> numericMappings_;
    std::unordered_multimap<std::string, detail::Mapping> textureMappings_;
    std::vector<std::pair<std::string, MatParamType>> params_;

public:
    explicit ShadeModel(const FilePath& path);

    std::shared_ptr<ShadeModelInput> createInput() const;

    size_t numParameters() const;
    void parameter(size_t i, std::string& name, MatParamType& type) const;

private:
    void loadImpl();
    void unloadImpl();
};

GF_NAMESPACE_END

#endif