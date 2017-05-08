#include "../scene/material.h"
#include "../scene/shademodel.h"
#include "../scene/materialparameter.h"
#include "../scene/texture.h"
#include "../render/renderstate.h"
#include "resource.h"
#include "logging.h"
#include "foundation/metaprop.h"
#include "foundation/exception.h"
#include <string>
#include <unordered_map>

GF_NAMESPACE_BEGIN

namespace
{
    bool toMatParamType(const std::string& s, MatParamType& out)
    {
        if (s == "float") out = MatParamType::_float;
        else if (s == "float4") out = MatParamType::_float4;
        else if (s == "float4x4") out = MatParamType::_float4x4;
        else if (s == "tex2d") out = MatParamType::_tex2d;
        else return false;
        return true;
    }
}

bool ShadeModel::loadImpl()
{
    MetaPropFile file;

    try
    {
        file.read(path().os);
    }
    catch (const Exception& e)
    {
        GF_LOG_WARN("Failed to load shade model {}. {}", path().os, e.msg());
        return false;
    }

    try
    {
        std::unordered_map<std::string, MatParamType> paramTypes;

        // @Parameters
        enforce<ShadeModelLoadException>(file.hasGroup("Parameters"), ".shade requires @Parameters.");
        const auto& Parameters = file.group("Parameters");

        for (int i = 0; Parameters.hasProp(std::to_string(i)); ++i)
        {
            const auto& prop = Parameters.prop(std::to_string(i));

            MatParamType type;
            const auto name = prop[1];
            enforce<ShadeModelLoadException>(toMatParamType(prop[0], type), "Invalid parameter type.");

            params_.emplace_back(name, type);
            paramTypes.emplace(name, type);
        }

        // ShaderStages
        const auto parseShaderStage = [&, this](ShaderType stageType, const std::string& stageName)
        {
            if (file.hasGroup(stageName))
            {
                const auto& SS = file.group(stageName);

                enforce<ShadeModelLoadException>(SS.hasProp("Compile"),
                    ".shade requires Compile: in @" + stageName + ").");
                const auto& Compile = SS.prop("Compile");

                const auto path = Compile[0];
                const auto entry = Compile[1];
                program_.compile(stageType, path, entry);

                for (int i = 0; SS.hasProp("Map" + std::to_string(i)); ++i)
                {
                    const auto& Map = SS.prop("Map" + std::to_string(i));
                    const auto param = Map[0];
                    const auto mapTo = Map[1];

                    enforce<ShadeModelLoadException>(paramTypes.find(param) != std::cend(paramTypes),
                        "Parameter " + param + " not found in Map:.");

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
    catch (const ResourceException& e)
    {
        GF_LOG_WARN("Failed to load shade model {}. {}", path().os, e.msg());
        unloadImpl();
        return false;
    }

    return true;
}

void ShadeModel::unloadImpl()
{
    params_.clear();
    textureMappings_.clear();
    numericMappings_.clear();
    program_ = ShaderProgram();
    drawCall_ = OptimizedDrawCall();
}

bool Material::loadImpl()
{
    MetaPropFile file;

    try
    {
        file.read(path().os);
    }
    catch (const Exception& e)
    {
        GF_LOG_WARN("Failed to load material {}. {}", path().os, e.msg());
        return false;
    }

    try
    {
        // @Shade
        enforce<MaterialLoadException>(file.hasGroup("Shade"), ".material requires @Shade.");
        const auto& Shade = file.group("Shade");

        enforce<MaterialLoadException>(Shade.hasProp("Path"), ".material requires Path: in @Shade.");
        const auto shadeModelPath = Shade.prop("Path")[0];

        const auto model = resourceManager.template obtain<ShadeModel>(shadeModelPath);
        model->load();
        enforce<ShadeModelLoadException>(model->ready(), "Failed to load .shade.");
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

            enforce<MaterialLoadException>(Shade.hasProp(name),
                ".material requires parameter value " + name + " in @Shade.");
            const auto& prop = Shade.prop(name);

            if (isNumeric(type))
            {
                if (isFloating(type))
                {
                    const auto propSize = prop.size();
                    const auto floats = reinterpret_cast<float*>(holder.numeric.get());
                    for (size_t n = 0; n < sizeofMatParam(type) / sizeof(float) && n < propSize; ++n)
                    {
                        floats[n] = prop.getFloat(n);
                    }
                }
                shadeModelIn_->updateNumeric(name, holder.numeric.get(), sizeofMatParam(type));
            }
            else
            {
                const auto texPath = prop[0];
                const auto tex = resourceManager.template obtain<MediaTexture>(texPath);
                tex->load();
                enforce<TextureLoadException>(tex->ready(), "Failed to load texture.");
                holder.texture = tex;
                shadeModelIn_->updateTexture(name, tex->resource());
            }
        }
    }
    catch (const ResourceException& e)
    {
        GF_LOG_WARN("Failed to load material {}. {}", path().os, e.msg());
        unloadImpl();
        return false;
    }

    return true;
}

void Material::unloadImpl()
{
    shadeModel_ = ResourceInterface<ShadeModel>();
    shadeModelIn_.reset();
    params_.clear();
}

GF_NAMESPACE_END