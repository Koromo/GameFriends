#include "parser.h"
#include "foundation/metaprop.h"
#include "foundation/exception.h"
#include <fstream>
#include <iostream>

std::string forward(Context& context, const std::string& confirm = "")
{
    if (context.tokens.empty())
    {
        error(context, "Unexpected EOF.");
    }
    
    const auto current = context.tokens.front();
    context.tokens.pop();
    context.current = current;
    
    if (!confirm.empty() && current.s != confirm)
    {
        syntaxError(context);
    }

    return current.s;
}

void addIdentifier(Context& context)
{
    if (!isValidIndentifier(context.current.s))
    {
        identifierError(context);
    }
    const auto added = context.identifiers.emplace(context.current.s).second;
    if (!added)
    {
        conflictError(context);
    }
}

Parameter_t& addParameter(Context& context)
{
    addIdentifier(context);
    auto& p = context.paramTable[context.current.s];
    p.name = context.current.s;
    return p;
}

void reservedWord(Context& context, const std::string& name)
{
    if (!isValidIndentifier(name))
    {
        error(context, "Converter Bugged!! Invalid reserved word \'" + name + "\'.");
    }
    const auto added = context.identifiers.emplace(name).second;
    if (!added)
    {
        error(context, "Converter Bugged!! Reserved word conflict \'" + name + "\'.");
    }
}

struct Parser
{
    struct P_Base
    {
        virtual void parse(Context& context) = 0;
    };

    struct P_Config : P_Base
    {
        struct P_Config_Base
        {
            virtual void parse(Context& context) = 0;
        };

        struct P_type : P_Config_Base
        {
            void parse(Context& context)
            {
                forward(context, "=");
                const auto type = forward(context);
                if (!isConfigType(type))
                {
                    syntaxError(context);
                }
                context.confType = type;
                forward(context, ";");
            }
        } type;

        struct P_name : P_Config_Base
        {
            void parse(Context& context)
            {
                forward(context, "=");
                context.confName = forward(context);
                forward(context, ";");
            }
        } name;

        struct P_output : P_Config_Base
        {
            void parse(Context& context)
            {
                forward(context, "=");
                context.confOut = forward(context);
                forward(context, ";");
            }
        } output;

        void parse(Context& context)
        {
            static std::unordered_map<std::string, P_Config_Base*> parserMap = {
                { RW_TYPE, &type },
                { RW_NAME, &name },
                { RW_OUTPUT, &output }
            };

            forward(context, "{");
            for (auto token = forward(context); token != "}"; token = forward(context))
            {
                const auto it = parserMap.find(token);
                if (it == std::cend(parserMap))
                {
                    syntaxError(context);
                    return;
                }
                it->second->parse(context);
            }
        }
    } Config;

    struct P_Parameters : P_Base
    {
        struct P_Parameters_Base
        {
            virtual void parse(Context& context) = 0;

            std::vector<std::string> parseFloats(Context& context, size_t n)
            {
                std::vector<std::string> floats(n);

                forward(context, "{");
                for (size_t i = 0; i < n - 1; ++i)
                {
                    auto token = forward(context);
                    if (!isFloat(token))
                    {
                        syntaxError(context);
                    }
                    floats[i] = token;
                    forward(context, ",");
                }

                auto token = forward(context);
                if (!isFloat(token))
                {
                    syntaxError(context);
                }
                floats[n - 1] = token;
                forward(context, "}");

                return floats;
            }
        };

        struct P_float : P_Parameters_Base
        {
            void parse(Context& context)
            {
                forward(context);
                auto& param = addParameter(context);

                param.type = RW_FLOAT;

                auto token = forward(context);
                if (token == ";")
                {
                    param.value = { "0" };
                }
                else
                {
                    if (token != "=")
                    {
                        syntaxError(context);
                    }

                    auto token = forward(context);
                    if (!isFloat(token))
                    {
                        syntaxError(context);
                    }
                    param.value = { token };
                    forward(context, ";");
                }
            }
        } T_float;

        struct P_float4 : P_Parameters_Base
        {
            void parse(Context& context)
            {
                forward(context);
                auto& param = addParameter(context);

                param.type = RW_FLOAT4;

                auto token = forward(context);
                if (token == ";")
                {
                    param.value = { "0", "0", "0", "0" };
                }
                else
                {
                    if (token != "=")
                    {
                        syntaxError(context);
                    }
                    param.value = parseFloats(context, 4);
                    forward(context, ";");
                }
            }
        } T_float4;

        struct P_float4x4 : P_Parameters_Base
        {
            void parse(Context& context)
            {
                forward(context);
                auto& param = addParameter(context);

                param.type = RW_FLOAT4X4;

                auto token = forward(context);
                if (token == ";")
                {
                    param.value = {
                        "0", "0", "0", "0",
                        "0", "0", "0", "0",
                        "0", "0", "0", "0",
                        "0", "0", "0", "0"};
                }
                else
                {
                    if (token != "=")
                    {
                        syntaxError(context);
                    }
                    param.value = parseFloats(context, 16);
                    forward(context, ";");
                }
            }
        } T_float4x4;

        struct P_tex2d : P_Parameters_Base
        {
            void parse(Context& context)
            {
                forward(context);
                auto& param = addParameter(context);

                param.type = RW_FLOAT;

                auto token = forward(context);
                if (token == ";")
                {
                    param.value = { "" };
                }
                else
                {
                    if (token != "=")
                    {
                        syntaxError(context);
                    }
                    param.value = { forward(context) };
                    forward(context, ";");
                }
            }
        } T_tex2d;

        void parse(Context& context)
        {
            static std::unordered_map<std::string, P_Parameters_Base*> parserMap = {
                { RW_FLOAT, &T_float },
                { RW_FLOAT4, &T_float4 },
                { RW_FLOAT4X4, &T_float4x4 },
                { RW_TEX2D, &T_tex2d },
            };

            forward(context, "{");
            for (auto token = forward(context); token != "}"; token = forward(context))
            {
                const auto it = parserMap.find(token);
                if (it == std::cend(parserMap))
                {
                    syntaxError(context);
                    return;
                }
                it->second->parse(context);
            }
        }
    } Parameters;

    struct P_Shaders : P_Base
    {
        struct P_Ref
        {
            struct P_Ref_Base
            {
                virtual void parse(Context& context, ShaderReference_t& shader) = 0;
            };

            struct P_path : P_Ref_Base
            {
                void parse(Context& context, ShaderReference_t& shader)
                {
                    forward(context, "=");
                    shader.path = forward(context);
                    forward(context, ";");
                }
            } path;

            struct P_entry : P_Ref_Base
            {
                void parse(Context& context, ShaderReference_t& shader)
                {
                    forward(context, "=");
                    shader.entry = forward(context);
                    forward(context, ";");
                }
            } entry;

            struct P_map : P_Ref_Base
            {
                struct P_map_Base
                {
                    virtual void parse(Context& context, ParameterMapping& mapping) = 0;

                    std::vector<std::string> parseFloats(Context& context, size_t n)
                    {
                        std::vector<std::string> floats(n);

                        forward(context, "(");
                        for (size_t i = 0; i < n - 1; ++i)
                        {
                            auto token = forward(context);
                            if (!isFloat(token))
                            {
                                syntaxError(context);
                            }
                            floats[i] = token;
                            forward(context, ",");
                        }

                        auto token = forward(context);
                        if (!isFloat(token))
                        {
                            syntaxError(context);
                        }
                        floats[n - 1] = token;
                        forward(context, ")");

                        return floats;
                    }
                };

                struct P_float : P_map_Base
                {
                    void parse(Context& context, ParameterMapping& mapping)
                    {
                        Parameter_t param;

                        forward(context, "(");
                        const auto val = forward(context);
                        if (!isFloat(val))
                        {
                            syntaxError(context);
                        }

                        param.type = RW_FLOAT;
                        param.value = { val };
                        forward(context, ")");
                        mapping.rawValue = param;
                    }
                } T_float;

                struct P_float4 : P_map_Base
                {
                    void parse(Context& context, ParameterMapping& mapping)
                    {
                        Parameter_t param;
                        param.type = RW_FLOAT4;
                        param.value = parseFloats(context, 4);
                        mapping.rawValue = param;
                    }
                } T_float4;

                struct P_float4x4 : P_map_Base
                {
                    void parse(Context& context, ParameterMapping& mapping)
                    {
                        Parameter_t param;
                        param.type = RW_FLOAT4X4;
                        param.value = parseFloats(context, 16);
                        mapping.rawValue = param;
                    }
                } T_float4x4;

                struct P_tex2d : P_map_Base
                {
                    void parse(Context& context, ParameterMapping& mapping)
                    {
                        Parameter_t param;

                        forward(context, "(");
                        const auto val = forward(context);
                        param.type = RW_TEX2D;
                        param.value = { val };
                        forward(context, ")");
                        mapping.rawValue = param;
                    }
                } T_tex2d;

                void parse(Context& context, ShaderReference_t& shader)
                {
                    static std::unordered_map<std::string, P_map_Base*> parserMap = {
                        { RW_FLOAT, &T_float },
                        { RW_FLOAT4, &T_float4 },
                        { RW_FLOAT4X4, &T_float4x4 },
                        { RW_TEX2D, &T_tex2d },
                    };

                    ParameterMapping mapping;

                    forward(context, "(");
                    const auto token = forward(context);
                    if (isParameterType(token))
                    {
                        const auto it = parserMap.find(token);
                        if (it == std::cend(parserMap))
                        {
                            syntaxError(context);
                            return;
                        }
                        it->second->parse(context, mapping);

                        mapping.isRawValue = true;
                        mapping.rawValue.name = "_lambda" + std::to_string(generateUniqueID());
                    }
                    else
                    {
                        mapping.param = token;
                        mapping.isRawValue = false;

                    }

                    forward(context, ",");
                    mapping.mapTo = forward(context);
                    shader.mappings.emplace_back(mapping);
                    forward(context, ")");
                    forward(context, ";");
                }
            } map;

            void parse(Context& context, ShaderReference_t& shader)
            {
                static std::unordered_map<std::string, P_Ref_Base*> parserMap = {
                    { RW_PATH, &path },
                    { RW_ENTRY, &entry },
                    { RW_MAP, &map }
                };

                forward(context, "{");
                for (auto token = forward(context); token != "}"; token = forward(context))
                {
                    const auto it = parserMap.find(token);
                    if (it == std::cend(parserMap))
                    {
                        syntaxError(context);
                        return;
                    }
                    it->second->parse(context, shader);
                }
            }
        } Ref;

        void parse(Context& context)
        {
            forward(context, "{");
            for (auto token = forward(context); token != "}"; token = forward(context))
            {
                if (token != RW_REF)
                {
                    syntaxError(context);
                }
                const auto refName = forward(context);
                addIdentifier(context);
                ShaderReference_t shader = {};
                Ref.parse(context, shader);
                context.shaders.emplace(refName, shader);
            }
        }
    } Shaders;

    struct P_Pass : P_Base
    {
        struct P_Pass_Base
        {
            virtual void parse(Context& context, Pass_t& pass) = 0;
        };

        struct P_depthTest : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                forward(context);
                pass.depthEnable = toBool(context);
                forward(context, ";");
            }
        } depthTest;

        struct P_depthFun : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                forward(context);
                pass.depthFun = toComparisonFun(context);
                forward(context, ";");
            }
        } depthFun;

        struct P_fillMode : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                forward(context);
                pass.fill = toFillMode(context);
                forward(context, ";");
            }
        } fillMode;

        struct P_cullFace : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                forward(context);
                pass.cull = toCullFace(context);
                forward(context, ";");
            }
        } cullFace;

        struct P_depthClip : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                forward(context);
                pass.depthClip = toBool(context);
                forward(context, ";");
            }
        } depthClip;

        struct P_vertexShader : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                pass.vs = forward(context);
                forward(context, ";");
            }
        } vertexShader;

        struct P_geometryShader : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                pass.gs = forward(context);
                forward(context, ";");
            }
        } geometryShader;

        struct P_pixelShader : P_Pass_Base
        {
            void parse(Context& context, Pass_t& pass)
            {
                forward(context, "=");
                pass.ps = forward(context);
                forward(context, ";");
            }
        } pixelShader;

        void parse(Context& context)
        {
            static std::unordered_map<std::string, P_Pass_Base*> parserMap = {
                { RW_DEPTHTEST, &depthTest },
                { RW_DEPTHFUN, &depthFun },
                { RW_FILLMODE, &fillMode },
                { RW_CULLFACE, &cullFace },
                { RW_DEPTHCLIP, &depthClip },
                { RW_VERTEXSHADER, &vertexShader },
                { RW_GEOMETRYSHADER, &geometryShader },
                { RW_PIXELSHADER, &pixelShader }
            };

            forward(context, "{");
            for (auto token = forward(context); token != "}"; token = forward(context))
            {
                const auto it = parserMap.find(token);
                if (it == std::cend(parserMap))
                {
                    syntaxError(context);
                    return;
                }
                it->second->parse(context, context.pass);
            }
        }
    } Pass;

    void operator()(Context& context)
    {
        static std::unordered_map<std::string, P_Base*> parserMap = {
            { RW_CONFIG, &Config },
            { RW_PARAMETERS, &Parameters },
            { RW_SHADERS, &Shaders },
            { RW_PASS, &Pass }
        };

        while (!context.tokens.empty())
        {
            const auto token = forward(context);
            const auto it = parserMap.find(token);
            if (it == std::cend(parserMap))
            {
                syntaxError(context);
            }
            it->second->parse(context);
        }
    }
} parse;

size_t stepByNonspace(size_t p, const std::string& str)
{
    while (p < str.length() && std::isspace(str[p]))
    {
        ++p;
    }
    return p;
}

void tokenizeLine(const std::string& line, size_t lineNumber, Context& context)
{
    static const std::string DERIMITERS = " \t,(){};=";

    for (size_t p = stepByNonspace(0, line); p < line.length(); p = stepByNonspace(p, line))
    {
        auto q = line.find_first_of(DERIMITERS, p);
        if (q == std::string::npos)
        {
            q = line.length();
        }

        std::string token;

        if (p == q)
        {
            token = line.substr(p, 1);
            p += 1;
        }
        else
        {
            token = line.substr(p, q - p);
            p = q;
        }

        context.tokens.push(Token{ token, lineNumber });
    }
}

void tokenize(std::ifstream& stream, Context& context)
{
    size_t numLines = 0;
    bool commentOut = false;

    while (!stream.eof())
    {
        ++numLines;
        std::string line;
        std::getline(stream, line);

        if (commentOut)
        {
            const auto p = line.find("*/");
            if (p == std::string::npos)
            {
                line.clear();
            }
            else
            {
                line = line.substr(p + 2);
                commentOut = false;
            }
        }

        std::string tmp = line;
        line.clear();
        while (!tmp.empty())
        {
            auto p = tmp.find("/*");
            if (p == std::string::npos)
            {
                line += tmp;
                tmp.clear();
            }
            else
            {
                line += tmp.substr(0, p);
                tmp = tmp.substr(p + 2);
                commentOut = true;
            }

            auto q = tmp.find("*/");
            if (q == std::string::npos)
            {
                tmp.clear();
            }
            else
            {
                tmp = tmp.substr(q + 2);
                commentOut = false;
            }
        }

        line = line.substr(0, line.find("//"));
        tokenizeLine(line, numLines, context);
    }
}

void initReservedWords(Context& context)
{
    reservedWord(context, RW_CONFIG);
    reservedWord(context, RW_PARAMETERS);
    reservedWord(context, RW_SHADERS);
    reservedWord(context, RW_REF);
    reservedWord(context, RW_PASS);

    reservedWord(context, RW_SHADE);
    reservedWord(context, RW_MATERIAL);
    reservedWord(context, RW_TYPE);
    reservedWord(context, RW_NAME);
    reservedWord(context, RW_OUTPUT);

    reservedWord(context, RW_TRUE);
    reservedWord(context, RW_FALSE);

    reservedWord(context, RW_FLOAT);
    reservedWord(context, RW_FLOAT4);
    reservedWord(context, RW_FLOAT4X4);
    reservedWord(context, RW_TEX2D);

    reservedWord(context, RW_PATH);
    reservedWord(context, RW_ENTRY);
    reservedWord(context, RW_MAP);

    reservedWord(context, RW_VERTEXSHADER);
    reservedWord(context, RW_GEOMETRYSHADER);
    reservedWord(context, RW_PIXELSHADER);
    reservedWord(context, RW_DEPTHTEST);
    reservedWord(context, RW_DEPTHFUN);
    reservedWord(context, RW_FILLMODE);
    reservedWord(context, RW_CULLFACE);
    reservedWord(context, RW_DEPTHCLIP);

    reservedWord(context, RW_NEVER);
    reservedWord(context, RW_LESS);
    reservedWord(context, RW_EQUAL);
    reservedWord(context, RW_LESSEQUAL);
    reservedWord(context, RW_GREATER);
    reservedWord(context, RW_NOTEQUAL);

    reservedWord(context, RW_GREATEREQUAL);
    reservedWord(context, RW_ALWAYS);
    reservedWord(context, RW_WIREFRAME);
    reservedWord(context, RW_SOLID);
    reservedWord(context, RW_NONE);
    reservedWord(context, RW_FRONT);
    reservedWord(context, RW_BACK);
}

void checkConfig(Context& context)
{
    if (context.confType.empty() || context.confName.empty() || context.confOut.empty())
    {
        error(context, ".matcode needs to describe Configs.");
    }
}

void checkShaderMappings(Context& context)
{
    for (const auto& shader : context.shaders)
    {
        for (const auto& mapping : shader.second.mappings)
        {
            if (!mapping.isRawValue)
            {
                const auto p = context.paramTable.find(mapping.param);
                if (p == std::cend(context.paramTable))
                {
                    error(context, "At \'" + shader.first + "\', a material parameter \'" + mapping.param + "\' is not exists.");
                }
            }
        }
    }
}

void checkShaderStages(Context& context)
{
    auto& pass = context.pass;

    const auto notFound = std::cend(context.shaders);
    const auto vs = context.shaders.find(pass.vs);
    const auto gs = context.shaders.find(pass.gs);
    const auto ps = context.shaders.find(pass.ps);

    if (!pass.vs.empty() && vs == notFound)
    {
        error(context, "At pass, a shader \'" + pass.vs + "\' is not exists.");
    }
    if (!pass.gs.empty() && gs == notFound)
    {
        error(context, "At pass, a shader \'" + pass.gs + "\' is not exists.");
    }
    if (!pass.ps.empty() && ps == notFound)
    {
        error(context, "At pass, a shader \'" + pass.ps + "\' is not exists.");
    }
}

void exportShade(const Context& context)
{
    int i;

    // @_Header
    MetaPropFile mpFile;
    mpFile.setAuther("MaterialConverter");
    mpFile.setComment("Generated from " + context.path);

    // @Parameters
    MetaPropGroup Parameters("Parameters");
    i = 0;

    for (const auto& param : context.paramTable)
    {
        MetaProperty prop(std::to_string(i));
        prop[0] = param.second.type;
        prop[1] = param.second.name;
        Parameters.addProp(prop);
        ++i;
    }

    for (const auto& shader : context.shaders)
    {
        for (const auto& mapping : shader.second.mappings)
        {
            if (mapping.isRawValue)
            {
                MetaProperty prop(std::to_string(i));
                prop[0] = mapping.rawValue.type;
                prop[1] = mapping.rawValue.name;
                Parameters.addProp(prop);
                ++i;
            }
        }
    }

    mpFile.addGroup(Parameters);

    // ShadersStages
    const auto buildShaderStage = [&](const ShaderReference_t& shaderRef, const std::string& stageName)
    {
        MetaPropGroup S(stageName);

        MetaProperty Compile("Compile");
        Compile[0] = shaderRef.path;
        Compile[1] = shaderRef.entry;

        S.addProp(Compile);

        i = 0;
        for (const auto& mapping : shaderRef.mappings)
        {
            MetaProperty Map("Map" + std::to_string(i));

            std::string paramName;
            if (mapping.isRawValue)
            {
                paramName = mapping.rawValue.name;
            }
            else
            {
                paramName = mapping.param;
            }

            Map[0] = paramName;
            Map[1] = mapping.mapTo;

            S.addProp(Map);
            ++i;
        }

        mpFile.addGroup(S);
    };

    if (!context.pass.vs.empty())
    {
        const auto s = context.shaders.find(context.pass.vs);
        buildShaderStage(s->second, "VS");
    }
    if (!context.pass.gs.empty())
    {
        const auto s = context.shaders.find(context.pass.gs);
        buildShaderStage(s->second, "GS");
    }
    if (!context.pass.ps.empty())
    {
        const auto s = context.shaders.find(context.pass.ps);
        buildShaderStage(s->second, "PS");
    }

    // @DepthStencil
    MetaPropGroup DepthStencil("DepthStencil");
    MetaProperty DepthEnable("DepthEnable");
    MetaProperty DepthFun("DepthFun");

    DepthEnable.set(0, context.pass.depthEnable);
    DepthFun.set(0, context.pass.depthFun);

    DepthStencil.addProp(DepthEnable);
    DepthStencil.addProp(DepthFun);
    mpFile.addGroup(DepthStencil);

    // @Rasterizer
    MetaPropGroup Rasterizer("Rasterizer");
    MetaProperty Fill("Fill");
    MetaProperty Cull("Cull");
    MetaProperty DepthClip("DepthClip");

    Fill.set(0, context.pass.fill);
    Cull.set(0, context.pass.cull);
    DepthClip.set(0, context.pass.depthClip);

    Rasterizer.addProp(Fill);
    Rasterizer.addProp(Cull);
    Rasterizer.addProp(DepthClip);
    mpFile.addGroup(Rasterizer);

    mpFile.write(context.confOut + "/" + context.confName + ".shade");
}

void exportMaterial(const Context& context)
{
    int i;

    // @_Header
    MetaPropFile mpFile;
    mpFile.setAuther("MaterialConverter");
    mpFile.setComment("Generated from " + context.path);

    // @Shade
    MetaPropGroup Shade("Shade");
    i = 0;

    for (const auto& param : context.paramTable)
    {
        MetaProperty prop(param.second.name);
        for (size_t j = 0; j < param.second.value.size(); ++j)
        {
            prop[j] = param.second.value[j];
        }
        Shade.addProp(prop);
        ++i;
    }

    for (const auto& shader : context.shaders)
    {
        for (const auto& mapping : shader.second.mappings)
        {
            if (mapping.isRawValue)
            {
                MetaProperty prop(mapping.rawValue.name);
                for (size_t j = 0; j < mapping.rawValue.value.size(); ++j)
                {
                    prop[j] = mapping.rawValue.value[j];
                }
                Shade.addProp(prop);
                ++i;
            }
        }
    }

    mpFile.addGroup(Shade);
    mpFile.write(context.confOut + "/" + context.confName + ".material");
}

int main(int argv, char** argc)
{
    const std::string messagePath = "MaterialConverterMsg.txt";
    std::ofstream message(messagePath);
    if (!message.is_open())
    {
        return 14; // Message file not opened
    }
    GF_SCOPE_EXIT{ message.close(); };

    if (argv != 2)
    {
        message << "Usage: " << argc[0] << " path(.matcode)" << std::endl;
        return 12; // Usage error exit
    }

    try
    {
        const std::string inputPath = argc[1];
        std::ifstream input(inputPath);
        if (!input.is_open())
        {
            message << "File not found (" << inputPath << ")." << std::endl;
            return 10; // Failed to open input
        }

        Context context = {};
        tokenize(input, context);
        input.close();

        context.path = inputPath;
        context.current.n = 0;
        initReservedWords(context);

        if (!context.errors.empty())
        {
            message << "System error!" << std::endl;
            for (const auto& e : context.errors)
            {
                message << e << std::endl;
            }
            return 8; // System error exit
        }

        parse(context);

        if (!context.errors.empty())
        {
            for (const auto& e : context.errors)
            {
                message << e << std::endl;
            }
            return 1; // Parse error exit
        }

        checkConfig(context);
        checkShaderMappings(context);
        checkShaderStages(context);

        if (!context.errors.empty())
        {
            for (const auto& e : context.errors)
            {
                message << e << std::endl;
            }
            return 2; // Consistency error exit
        }

        // Now, we can create output file.
        // First, create .shade
        exportShade(context);

        // Finaly create .material
        if (context.confType == RW_MATERIAL)
        {
            exportMaterial(context);
        }
    }
    catch (const std::exception& e)
    {
        message << e.what() << std::endl;
        return 5; // Any error
    }
}