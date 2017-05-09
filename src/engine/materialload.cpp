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
#include <vector>

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
        enforce<ShadeModelLoadException>(file.has("Parameters"), ".shade requires @Parameters.");
        const auto& Parameters = file.get("Parameters");

        for (int i = 0; Parameters.has(std::to_string(i)); ++i)
        {
            const auto& prop = Parameters.get(std::to_string(i));
            enforce<ShadeModelLoadException>(prop.size() >= 2, "parameter requires <type> <name>.");

            MatParamType type;
            const auto name = prop[1];
            enforce<ShadeModelLoadException>(toMatParamType(prop[0], type), "Invalid parameter type.");

            params_.emplace_back(name, type);
            paramTypes.emplace(name, type);
        }

        // ShaderStages
        const auto parseShaderStage = [&, this](ShaderType stageType, const std::string& stageName)
        {
            if (file.has(stageName))
            {
                const auto& SS = file.get(stageName);

                enforce<ShadeModelLoadException>(SS.has("Compile") && SS.get("Compile").size() >= 2,
                    ".shade requires Compile: <path> <entry> in @" + stageName + ").");
                const auto& Compile = SS.get("Compile");

                const auto path = Compile[0];
                const auto entry = Compile[1];

                std::vector<ShaderMacro> macros;
                if (SS.has("Macro"))
                {
                    const auto& Macro = SS.get("Macro");
                    enforce<ShadeModelLoadException>(Macro.size() % 2 == 0,
                        "Macro: requires (<name <value>)* in @" + stageName + ").");

                    const auto size = Macro.size();
                    for (size_t i = 0; i < size; i += 2)
                    {
                        macros.emplace_back(ShaderMacro{ Macro[i], Macro[i + 1] });
                    }
                }

                program_.compile(stageType, path, entry, std::cbegin(macros), std::cend(macros));

                for (int i = 0; SS.has("Map" + std::to_string(i)); ++i)
                {
                    const auto& Map = SS.get("Map" + std::to_string(i));
                    enforce<ShadeModelLoadException>(Compile.size() >= 2, "Map: requires <param> <map to> : in @" + stageName + ").");

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
        DepthState depthState = DepthState::DEFAULT;

        enforce<ShadeModelLoadException>(file.has("DepthStencil"), ".shade requires @DepthStencil.");
        const auto& DepthStencil = file.get("DepthStencil");

        enforce<ShadeModelLoadException>(DepthStencil.has("DepthEnable") && DepthStencil.get("DepthEnable").size() >= 1,
            "@DepthStencil requires DepthEnable: <bool>.");
        depthState.depthEnable = !!DepthStencil.get("DepthEnable").stoi(0);

        enforce<ShadeModelLoadException>(DepthStencil.has("DepthFun") && DepthStencil.get("DepthFun").size() >= 1,
            "@DepthStencil requires DepthFun: <fun>.");
        depthState.depthFun = static_cast<ComparisonFun>(DepthStencil.get("DepthFun").stoi(0));

        drawCall_.setDepthState(depthState);

        // @Rasterizer
        RasterizerState rasterizerState = RasterizerState::DEFAULT;

        enforce<ShadeModelLoadException>(file.has("Rasterizer"), ".shade requires @Rasterizer.");
        const auto& Rasterizer = file.get("Rasterizer");

        enforce<ShadeModelLoadException>(Rasterizer.has("Fill") && Rasterizer.get("Fill").size() >= 1,
            "@Rasterizer requires Fill: <fill>.");
        rasterizerState.fillMode = static_cast<FillMode>(Rasterizer.get("Fill").stoi(0));

        enforce<ShadeModelLoadException>(Rasterizer.has("Cull") && Rasterizer.get("Cull").size() >= 1,
            "@Rasterizer requires Cull: <cull>.");
        rasterizerState.cullFace = static_cast<CullingFace>(Rasterizer.get("Cull").stoi(0));

        enforce<ShadeModelLoadException>(Rasterizer.has("DepthClip") && Rasterizer.get("DepthClip").size() >= 1,
            "@Rasterizer requires DepthClip: <bool>.");
        rasterizerState.depthClip = !!Rasterizer.get("DepthClip").stoi(0);

        drawCall_.setRasterizerState(rasterizerState);
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
        enforce<MaterialLoadException>(file.has("Shade"), ".material requires @Shade.");
        const auto& Shade = file.get("Shade");

        enforce<MaterialLoadException>(Shade.has("Path") && Shade.get("Path").size() >= 1,
            ".material requires Path: in @Shade.");
        const auto shadeModelPath = Shade.get("Path")[0];

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

            enforce<MaterialLoadException>(Shade.has(name),
                ".material requires parameter value " + name + " in @Shade.");
            const auto& prop = Shade.get(name);

            if (isNumeric(type))
            {
                if (isFloating(type))
                {
                    const auto propSize = prop.size();
                    const auto floats = reinterpret_cast<float*>(holder.numeric.get());
                    for (size_t n = 0; n < sizeofMatParam(type) / sizeof(float) && n < propSize; ++n)
                    {
                        floats[n] = prop.stof(n);
                    }
                }
                shadeModelIn_->updateNumeric(name, holder.numeric.get(), sizeofMatParam(type));
            }
            else
            {
                enforce<MaterialLoadException>(prop.size() >= 1,
                    name + " requires texture path in @Shade.");
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