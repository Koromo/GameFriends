#include "../scene/material.h"
#include "../scene/shademodel.h"
#include "../scene/materialparameter.h"
#include "../scene/texture.h"
#include "../render/renderstate.h"
#include "resource.h"
#include "foundation/metaprop.h"
#include "foundation/exception.h"
#include <string>
#include <unordered_map>

GF_NAMESPACE_BEGIN

namespace
{
    MatParamType toMatParamType(const std::string& s)
    {
        if (s == "float") return MatParamType::_float;
        if (s == "float4") return MatParamType::_float4;
        if (s == "float4x4") return MatParamType::_float4x4;
        if (s == "tex2d") return MatParamType::_tex2d;
        check(false);
        return MatParamType::_float;
    }
}

void ShadeModel::loadImpl()
{
    MetaPropFile file;
    file.read(path().os);

    std::unordered_map<std::string, MatParamType> paramTypes;

    // @Parameters
    const auto& Parameters = file.group("Parameters");
    for (int i = 0; Parameters.hasProp(std::to_string(i)); ++i)
    {
        const auto& prop = Parameters.prop(std::to_string(i));
        const auto type = toMatParamType(prop[0]);
        const auto name = prop[1];
        params_.emplace_back(name, type);
        paramTypes.emplace(name, type);
    }

    // ShaderStages
    const auto parseShaderStage = [&](ShaderType stageType, const std::string& stageName)
    {
        if (file.hasGroup(stageName))
        {
            const auto& VS = file.group(stageName);

            const auto& Compile = VS.prop("Compile");
            const auto path = Compile[0];
            const auto entry = Compile[1];
            program_.compile(stageType, path, entry);

            for (int i = 0; VS.hasProp("Map" + std::to_string(i)); ++i)
            {
                const auto& Map = VS.prop("Map" + std::to_string(i));
                const auto param = Map[0];
                const auto mapTo = Map[1];
                check(paramTypes.find(param) != std::cend(paramTypes));

                const auto type = paramTypes[param];
                detail::Mapping mapping;
                mapping.maxSize = sizeofMatParam(type);
                mapping.toName = mapTo;
                mapping.toType = stageType;

                if (isNumeric(type))
                {
                    numericMappings_.emplace(param, mapping);
                }
                else
                {
                    textureMappings_.emplace(param, mapping);
                }
            }
        }
    };

    parseShaderStage(ShaderType::vertex, "VS");
    parseShaderStage(ShaderType::geometry, "GS");
    parseShaderStage(ShaderType::pixel, "PS");
    drawCall_.setShaders(program_);

    // @DepthStencil
    if (file.hasGroup("DepthStencil"))
    {
        DepthState state = DepthState::DEFAULT;
        const auto& DepthStencil = file.group("DepthStencil");
        
        if (DepthStencil.hasProp("DepthEnable"))
        {
            state.depthEnable = !!DepthStencil.prop("DepthEnable").getInt(0);
        }
        if (DepthStencil.hasProp("DepthFun"))
        {
            state.depthFun = static_cast<ComparisonFun>(DepthStencil.prop("DepthFun").getInt(0));
        }

        drawCall_.setDepthState(state);
    }

    // @Rasterizer
    if (file.hasGroup("Rasterizer"))
    {
        RasterizerState state = RasterizerState::DEFAULT;
        const auto& Rasterizer = file.group("Rasterizer");

        if (Rasterizer.hasProp("Fill"))
        {
            state.fillMode = static_cast<FillMode>(Rasterizer.prop("Fill").getInt(0));
        }
        if (Rasterizer.hasProp("Cull"))
        {
            state.cullFace = static_cast<CullingFace>(Rasterizer.prop("Cull").getInt(0));
        }
        if (Rasterizer.hasProp("DepthClip"))
        {
            state.depthClip = !!Rasterizer.prop("DepthClip").getInt(0);
        }

        drawCall_.setRasterizerState(state);
    }
}

void ShadeModel::unloadImpl()
{
    params_.clear();
    textureMappings_.clear();
    numericMappings_.clear();
    program_ = ShaderProgram();
    drawCall_ = OptimizedDrawCall();
}

void Material::loadImpl()
{
    MetaPropFile file;
    file.read(path().os);

    // @Shade
    const auto& Shade = file.group("Shade");
    const auto shadeModelPath = Shade.prop("Path")[0];

    const auto model = resourceTable.template obtain<ShadeModel>(shadeModelPath);
    model->load();
    shadeModel_ = model;

    const auto numModelParams = model->numParameters();
    for (size_t i = 0; i < numModelParams; ++i)
    {
        std::string name;
        MatParamType type;
        model->parameter(i, name, type);

        ParamHolder holder;
        holder.type = type;

        if (isNumeric(type))
        {
            holder.numeric.reset(new char[sizeofMatParam(type)]);
        }
        params_.emplace(name, holder);
    }

    shadeModelIn_ = model->createInput();
    for (size_t i = 0; i < numModelParams; ++i)
    {
        std::string name;
        MatParamType type;
        model->parameter(i, name, type);

        auto& holder = params_[name];
        const auto& prop = Shade.prop(name);

        if (isNumeric(type))
        {
            if (isFloating(type))
            {
                const auto propSize = prop.size();
                check(sizeof(float) * propSize <= sizeofMatParam(type));
                const auto floats = reinterpret_cast<float*>(holder.numeric.get());
                for (size_t n = 0; n < propSize; ++n)
                {
                    floats[n] = prop.getFloat(n);
                }
            }
            shadeModelIn_->updateNumeric(name, holder.numeric.get(), sizeofMatParam(type));
        }
        else
        {
            const auto texPath = prop[0];
            const auto tex = resourceTable.template obtain<MediaTexture>(texPath);
            tex->load();
            holder.texture = tex;
            shadeModelIn_->updateTexture(name, tex->resource());
        }
    }
}

void Material::unloadImpl()
{
    shadeModel_ = ResourceInterface<ShadeModel>();
    shadeModelIn_.reset();
    params_.clear();
}

GF_NAMESPACE_END