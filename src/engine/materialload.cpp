#include "../scene/material.h"
#include "../render/vertexdata.h"
#include "../render/renderstate.h"
#include "../render/drawcall.h"
#include "../render/shaderprogram.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <queue>
#include <fstream>
#include <cctype>

GF_NAMESPACE_BEGIN

namespace
{
    struct Parameter
    {
        ParameterType type;
        std::string name;
    };

    struct Parameters
    {
        std::unordered_map<std::string, Parameter> table;
    };

    struct ParameterMapping
    {
        std::string param;
        std::string mapTo;
    };

    struct ReferenceShader
    {
        std::string path;
        std::string entry;
        std::vector<ParameterMapping> mapping;
    };

    struct PassSetup
    {
        size_t priority;

        DepthState ds = DepthState::DEFAULT;
        RasterizerState rs = RasterizerState::DEFAULT;

        std::string vs;
        std::string gs;
        std::string ps;
    };

    struct Token
    {
        std::string token;
        size_t line;
    };

    struct Context
    {
        std::string path;

        std::queue<Token> tokens;
        Token current;

        std::unordered_set<std::string> identifiers;
        Parameters params;
        Parameters autoParams;
        std::unordered_map<std::string, ReferenceShader> shaders;
        std::vector<PassSetup> passes;
    };

    void error(const Context& context, const std::string& msg)
    {
        throw MaterialLoadException(
            msg + "\n" +
            "FILE: " + context.path + "\n" +
            "LINE: " + std::to_string(context.current.line));
    }

    void syntaxError(const Context& context, const std::string& syntax)
    {
        error(context, "Syntax error : Unexpected token \'" + syntax + "\'.");
    }

    std::string forward(Context& context, const std::string& except = "")
    {
        if (context.tokens.empty())
        {
            error(context, "Unexpected EOF.");
        }

        const auto current = context.tokens.front();
        context.tokens.pop();
        context.current = current;

        if (!except.empty() && current.token != except)
        {
            syntaxError(context, current.token);
        }

        return current.token;
    }

    void addIdentifier(Context& context, const std::string& name)
    {
        const auto added = context.identifiers.emplace(name).second;
        if (!added)
        {
            syntaxError(context, name);
        }
    }

    void addParameter(Context& context, ParameterType type, const std::string& name)
    {
        addIdentifier(context, name);
        context.params.table.emplace(name, Parameter{ type, name });
    }

    void addAutoParameter(Context& context, ParameterType type, const std::string& name)
    {
        addIdentifier(context, name);
        context.autoParams.table.emplace(name, Parameter{ type, name });
    }

    // Block
    const std::string RW_PARAMETERS = "Parameters";
    const std::string RW_SHADERS = "Shaders";
    const std::string RW_REF = "Ref";
    const std::string RW_PASS = "Pass";

    // generic word
    const std::string RW_TRUE = "true";
    const std::string RW_FALSE = "false";

    // type
    const std::string RW_FLOAT = "float";
    const std::string RW_FLOAT4 = "float4";
    const std::string RW_FLOAT4X4 = "float4x4";
    const std::string RW_TEX2D = "tex2d";

    // shader word
    const std::string RW_PATH = "path";
    const std::string RW_ENTRY = "entry";
    const std::string RW_MAP = "map";

    // pipeline word
    const std::string RW_VERTEXSHADER = "vertexShader";
    const std::string RW_GEOMETRYSHADER = "geometryShader";
    const std::string RW_PIXELSHADER = "pixelShader";
    const std::string RW_DEPTHTEST = "depthTest";
    const std::string RW_DEPTHFUN = "depthFun";
    const std::string RW_FILLMODE = "fillMode";
    const std::string RW_CULLFACE = "cullFace";
    const std::string RW_DEPTHCLIP = "depthClip";

    // pipeline value word
    const std::string RW_NEVER = "never";
    const std::string RW_LESS = "less";
    const std::string RW_EQUAL = "equal";
    const std::string RW_LESSEQUAL = "lessEqual";
    const std::string RW_GREATER = "greater";
    const std::string RW_NOTEQUAL = "notEqual";
    const std::string RW_GREATEREQUAL = "greaterEqual";
    const std::string RW_ALWAYS = "always";
    const std::string RW_WIREFRAME = "wireframe";
    const std::string RW_SOLID = "solid";
    const std::string RW_NONE = "none";
    const std::string RW_FRONT = "front";
    const std::string RW_BACK = "back";

    bool stob(const Context& context, const std::string& s)
    {
        if (s == "true")
        {
            return true;
        }
        else if (s == "false")
        {
            return false;
        }
        syntaxError(context, s);
        return true;
    }

    ParameterType toParameterType(const Context& context, const std::string& s)
    {
        if (s == RW_FLOAT) return ParameterType::_float;
        if (s == RW_FLOAT4) return ParameterType::_float4;
        if (s == RW_FLOAT4X4) return ParameterType::_float4x4;
        if (s == RW_TEX2D) return ParameterType::_tex2d;
        syntaxError(context, s);
        return ParameterType::_float;
    }

    ComparisonFun toComparisonFun(const Context& context, const std::string& s)
    {
        if (s == RW_NEVER) return ComparisonFun::never;
        if (s == RW_LESS) return ComparisonFun::less;
        if (s == RW_EQUAL) return ComparisonFun::equal;
        if (s == RW_LESSEQUAL) return ComparisonFun::lessEqual;
        if (s == RW_GREATER) return ComparisonFun::greater;
        if (s == RW_NOTEQUAL) return ComparisonFun::notEqual;
        if (s == RW_GREATEREQUAL) return ComparisonFun::greaterEqual;
        if (s == RW_ALWAYS) return ComparisonFun::always;
        syntaxError(context, s);
        return ComparisonFun::always;
    }

    FillMode toFillMode(const Context& context, const std::string& s)
    {
        if (s == RW_WIREFRAME) return FillMode::wireframe;
        if (s == RW_SOLID) return FillMode::solid;
        syntaxError(context, s);
        return FillMode::solid;
    }

    CullingFace toCullFace(const Context& context, const std::string& s)
    {
        if (s == RW_NONE) return CullingFace::none;
        if (s == RW_FRONT) return CullingFace::front;
        if (s == RW_BACK) return CullingFace::back;
        syntaxError(context, s);
        return CullingFace::none;
    }

    struct Parser
    {
        struct Base_t
        {
            virtual void parse(Context& context) = 0;
        };

        struct Parameters_t : Base_t
        {
            void parse(Context& context)
            {
                forward(context, "{");
                for (auto token = forward(context); token != "}"; token = forward(context))
                {
                    const auto type = toParameterType(context, token);
                    const auto name = forward(context);
                    addParameter(context, type, name);
                    forward(context, ";");
                }
            }
        } Parameters;

        struct Shaders_t : Base_t
        {
            struct Ref_t
            {
                struct Ref_Base_t
                {
                    virtual void parse(Context& context, ReferenceShader& ref) = 0;
                };

                struct path_t : Ref_Base_t
                {
                    void parse(Context& context, ReferenceShader& ref)
                    {
                        forward(context, "=");
                        ref.path = forward(context);
                        forward(context, ";");
                    }
                } path;

                struct entry_t : Ref_Base_t
                {
                    void parse(Context& context, ReferenceShader& ref)
                    {
                        forward(context, "=");
                        ref.entry = forward(context);
                        forward(context, ";");
                    }
                } entry;

                struct map_t : Ref_Base_t
                {
                    void parse(Context& context, ReferenceShader& ref)
                    {
                        forward(context, "(");
                        const auto param = forward(context);
                        forward(context, ",");
                        const auto mapTo = forward(context);
                        ref.mapping.emplace_back(ParameterMapping{ param, mapTo });
                        forward(context, ")");
                        forward(context, ";");
                    }
                } map;

                void parse(Context& context, ReferenceShader& ref)
                {
                    static std::unordered_map<std::string, Ref_Base_t*> parserMap = {
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
                            syntaxError(context, token);
                        }
                        it->second->parse(context, ref);
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
                        syntaxError(context, token);
                    }
                    const auto refName = forward(context);
                    addIdentifier(context, refName);
                    ReferenceShader ref = {};
                    Ref.parse(context, ref);
                    context.shaders.emplace(refName, ref);
                }
            }
        } Shaders;

        struct Pass_t : Base_t
        {
            struct Pass_Base_t
            {
                virtual void parse(Context& context, PassSetup& pass) = 0;
            };

            struct depthTest_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    const auto s = forward(context);
                    pass.ds.depthTest = stob(context, s);
                    forward(context, ";");
                }
            } depthTest;

            struct depthFun_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    const auto s = forward(context);
                    pass.ds.depthFun = toComparisonFun(context, s);
                    forward(context, ";");
                }
            } depthFun;

            struct fillMode_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    const auto& s = forward(context);
                    pass.rs.fillMode = toFillMode(context, s);
                    forward(context, ";");
                }
            } fillMode;

            struct cullFace_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    const auto& s = forward(context);
                    pass.rs.cullFace = toCullFace(context, s);
                    forward(context, ";");
                }
            } cullFace;

            struct depthClip_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    const auto& s = forward(context);
                    pass.rs.depthClip = stob(context, s);
                    forward(context, ";");
                }
            } depthClip;

            struct vertexShader_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    pass.vs = forward(context); 
                    forward(context, ";");
                }
            } vertexShader;

            struct geometryShader_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    pass.gs = forward(context);
                    forward(context, ";");
                }
            } geometryShader;

            struct pixelShader_t : Pass_Base_t
            {
                void parse(Context& context, PassSetup& pass)
                {
                    forward(context, "=");
                    pass.ps = forward(context);
                    forward(context, ";");
                }
            } pixelShader;

            void parse(Context& context)
            {
                static std::unordered_map<std::string, Pass_Base_t*> parserMap = {
                    { RW_DEPTHTEST, &depthTest },
                    { RW_DEPTHFUN, &depthFun },
                    { RW_FILLMODE, &fillMode },
                    { RW_CULLFACE, &cullFace },
                    { RW_DEPTHCLIP, &depthClip },
                    { RW_VERTEXSHADER, &vertexShader },
                    { RW_GEOMETRYSHADER, &geometryShader },
                    { RW_PIXELSHADER, &pixelShader }
                };

                PassSetup pass = {};
                auto priority = forward(context);
                pass.priority = std::stoi(priority);

                forward(context, "{");
                for (auto token = forward(context); token != "}"; token = forward(context))
                {
                    const auto it = parserMap.find(token);
                    if (it == std::cend(parserMap))
                    {
                        syntaxError(context, token);
                    }
                    it->second->parse(context, pass);
                }

                context.passes.emplace_back(pass);
            }
        } Pass;

        void operator()(Context& context)
        {
            static std::unordered_map<std::string, Base_t*> parserMap = {
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
                    syntaxError(context, token);
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
        static const std::string DERIMITERS = " \t,(){};";

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
                p = q + 1;
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
            while (1)
            {
                auto p = tmp.find("/*");
                if (p == std::string::npos)
                {
                    line += tmp;
                    break;
                }
                line += tmp.substr(0, p);
                tmp = tmp.substr(p + 2);
                auto q = tmp.find("*/");
                if (q == std::string::npos)
                {
                    commentOut = true;
                    break;
                }
                tmp = tmp.substr(q + 2);
            }

            line = line.substr(0, line.find("//"));
            tokenizeLine(line, numLines, context);
        }
    }

    void initContext(Context& context)
    {
        addIdentifier(context, RW_PARAMETERS);
        addIdentifier(context, RW_SHADERS);
        addIdentifier(context, RW_REF);
        addIdentifier(context, RW_PASS);

        addIdentifier(context, RW_TRUE);
        addIdentifier(context, RW_FALSE);

        addIdentifier(context, RW_FLOAT);
        addIdentifier(context, RW_FLOAT4);
        addIdentifier(context, RW_FLOAT4X4);
        addIdentifier(context, RW_TEX2D);

        addIdentifier(context, RW_PATH);
        addIdentifier(context, RW_ENTRY);
        addIdentifier(context, RW_MAP);

        addIdentifier(context, RW_VERTEXSHADER);
        addIdentifier(context, RW_GEOMETRYSHADER);
        addIdentifier(context, RW_PIXELSHADER);
        addIdentifier(context, RW_DEPTHTEST);
        addIdentifier(context, RW_DEPTHFUN);
        addIdentifier(context, RW_FILLMODE);
        addIdentifier(context, RW_CULLFACE);
        addIdentifier(context, RW_DEPTHCLIP);

        addIdentifier(context, RW_NEVER);
        addIdentifier(context, RW_LESS);
        addIdentifier(context, RW_EQUAL);
        addIdentifier(context, RW_LESSEQUAL);
        addIdentifier(context, RW_GREATER);
        addIdentifier(context, RW_NOTEQUAL);
        addIdentifier(context, RW_GREATEREQUAL);
        addIdentifier(context, RW_ALWAYS);
        addIdentifier(context, RW_WIREFRAME);
        addIdentifier(context, RW_SOLID);
        addIdentifier(context, RW_NONE);
        addIdentifier(context, RW_FRONT);
        addIdentifier(context, RW_BACK);

        addAutoParameter(context, ParameterType::_float4x4, AutoParameter::WORLD);
        addAutoParameter(context, ParameterType::_float4x4, AutoParameter::VIEW);
        addAutoParameter(context, ParameterType::_float4x4, AutoParameter::PROJ);
        addAutoParameter(context, ParameterType::_float4x4, AutoParameter::TRANS);
    }

    std::shared_ptr<Pass> createPass(const PassSetup& setup, const Context& context)
    {
        const auto pass = std::make_shared<Pass>();
        pass->setPriority(setup.priority);

        OptimizedDrawCall drawCall;
        drawCall.setDepthState(setup.ds);
        drawCall.setRasterizerState(setup.rs);
            
        const auto shaders = std::make_shared<ShaderProgram>();
        const auto shaderNotFound = std::cend(context.shaders);
        const auto vsSetupFound = context.shaders.find(setup.vs);
        const auto gsSetupFound = context.shaders.find(setup.gs);
        const auto psSetupFound = context.shaders.find(setup.ps);

        check(vsSetupFound != shaderNotFound);

        const auto& vsSetup = vsSetupFound->second;
        shaders->compile(ShaderType::vertex, vsSetup.path, vsSetup.entry);
        if (gsSetupFound != shaderNotFound)
        {
            const auto& gsSetup = gsSetupFound->second;
            shaders->compile(ShaderType::geometry, gsSetup.path, gsSetup.entry);
        }
        if (psSetupFound != shaderNotFound)
        {
            const auto& psSetup = psSetupFound->second;
            shaders->compile(ShaderType::pixel, psSetup.path, psSetup.entry);
        }

        drawCall.setShaders(*shaders);

        pass->setDrawCallBase(drawCall);
        pass->setShaderProgram(shaders);

        const auto paramNotFound = std::cend(context.params.table);
        const auto autoParamNotFound = std::cend(context.autoParams.table);

        const auto getParameter = [&](const std::string& name)
        {
            const auto p = context.params.table.find(name);
            if (p != std::cend(context.params.table))
            {
                return p->second;
            }
            const auto q = context.autoParams.table.find(name);
            if (q != std::cend(context.autoParams.table))
            {
                return q->second;
            }
            check(false);
            return Parameter();
        };

        for (const auto& mapping : vsSetup.mapping)
        {
            const auto p = getParameter(mapping.param);
            if (isNumericParameter(p.type))
            {
                pass->mapNumeric(mapping.param, ShaderType::vertex, mapping.mapTo);
            }
            else
            {
                pass->mapTexture(mapping.param, ShaderType::vertex, mapping.mapTo);
            }
        }
        if (gsSetupFound != shaderNotFound)
        {
            const auto& gsSetup = gsSetupFound->second;
            for (const auto& mapping : gsSetup.mapping)
            {
                const auto p = getParameter(mapping.param);
                if (isNumericParameter(p.type))
                {
                    pass->mapNumeric(mapping.param, ShaderType::geometry, mapping.mapTo);
                }
                else
                {
                    pass->mapTexture(mapping.param, ShaderType::geometry, mapping.mapTo);
                }
            }
        }
        if (psSetupFound != shaderNotFound)
        {
            const auto& psSetup = psSetupFound->second;
            for (const auto& mapping : psSetup.mapping)
            {
                const auto p = getParameter(mapping.param);
                if (isNumericParameter(p.type))
                {
                    pass->mapNumeric(mapping.param, ShaderType::pixel, mapping.mapTo);
                }
                else
                {
                    pass->mapTexture(mapping.param, ShaderType::pixel, mapping.mapTo);
                }
            }
        }

        return pass;
    }
}

void Material::loadImpl()
{
    if (!doLoading_)
    {
        return;
    }
    std::ifstream stream(path());
    enforce<FileException>(stream.is_open(), "File not found (" + path() + ").");

    Context context = {};
    tokenize(stream, context);
    stream.close();

    context.path = path();
    initContext(context);
    parse(context);

    for (const auto& param : context.params.table)
    {
        const auto type = param.second.type;
        const auto name = param.second.name;

        Param p;
        p.type = type;
        if (isNumericParameter(type))
        {
            p.numeric.reset(new char[sizeofParameter(type)]);
        }

        paramTable_.emplace(name, p);
    }
    for (const auto& passSetup : context.passes)
    {
        const auto pass = createPass(passSetup, context);
        passes_.insert(pass);
    }
}

void Material::unloadImpl()
{
    passes_.clear();
    paramTable_.clear();
}

GF_NAMESPACE_END