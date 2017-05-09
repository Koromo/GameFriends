#pragma once

#include "foundation/prerequest.h"
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <cctype>

using namespace GF_NAMESPACE;

// Block
const std::string RW_CONFIG = "Config";
const std::string RW_PARAMETERS = "Parameters";
const std::string RW_SHADERS = "Shaders";
const std::string RW_REF = "Ref";
const std::string RW_PASS = "Pass";

// config word
const std::string RW_SHADE = "shade";
const std::string RW_MATERIAL = "material";
const std::string RW_TYPE = "type";
const std::string RW_NAME = "name";
const std::string RW_OUTPUT = "output";

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

struct Token
{
    std::string s;
    size_t n;
};

struct Parameter_t
{
    std::string name;
    std::string type;
    std::vector<std::string> value;
};

struct ParameterMapping
{
    bool isRawValue;
    Parameter_t rawValue;
    std::string param;
    std::string mapTo;
};

struct ShaderReference_t
{
    std::string path;
    std::string entry;
    std::vector<ParameterMapping> mappings;
};

struct Pass_t
{
    std::string vs;
    std::string gs;
    std::string ps;

    int depthEnable = 1;
    int depthFun = 3;

    int fill = 1;
    int cull = 2;
    int depthClip = 1;
};

struct Context
{
    std::string path;

    std::queue<Token> tokens;
    Token current;

    std::string confType;
    std::string confName;
    std::string confOut;
    std::unordered_set<std::string> identifiers;
    std::unordered_map<std::string, Parameter_t> paramTable;
    std::unordered_map<std::string, ShaderReference_t> shaders;
    Pass_t pass;

    std::vector<std::string> errors;
};

inline size_t generateUniqueID()
{
    static unsigned i = 0;
    return i++;
}

inline void error(Context& context, const std::string& msg)
{
    context.errors.emplace_back(msg);
}

inline void syntaxError(Context& context)
{
    error(context, "Syntax error: Unexpected token \'" + context.current.s + "\' in "
        + context.path + std::to_string(context.current.n) + " line.");
}

inline void identifierError(Context& context)
{
    error(context, "Invalid indentifier: \'" + context.current.s + "\' in "
        + context.path + std::to_string(context.current.n) + " line.");
}

inline void conflictError(Context& context)
{
    error(context, "Indentifier conflict: \'" + context.current.s + "\' in "
        + context.path + std::to_string(context.current.n) + " line.");
}

inline bool isValidIndentifier(const std::string& name)
{
    if (name.find("_Header") != std::string::npos ||
        name.find("@") != std::string::npos ||
        name.find(":") != std::string::npos)
    {
        return false;
    }
    return true;
}

inline bool isFloat(const std::string& s)
{
    bool dot = false;
    for (size_t p = 0; p < s.length(); ++p)
    {
        if (s[p] == '.')
        {
            if (dot)
            {
                return false;
            }
            dot = true;
        }
        else if (!isdigit(s[p]))
        {
            return false;
        }
    }

    return true;
}

inline bool isConfigType(const std::string& s)
{
    return s == RW_SHADE || s == RW_MATERIAL;
}

inline bool isParameterType(const std::string& s)
{
    return s == RW_FLOAT || s == RW_FLOAT4 || s == RW_FLOAT4X4 || s == RW_TEX2D;
}

inline int toBool(Context& context)
{
    const auto& s = context.current.s;
    if (s == RW_TRUE) return 1;
    if(s == RW_FALSE) return 0;
    syntaxError(context);
    return -1;
}

inline int toComparisonFun(Context& context)
{
    const auto& s = context.current.s;
    if (s == RW_NEVER) return 0;
    if (s == RW_LESS) return 1;
    if (s == RW_EQUAL) return 2;
    if (s == RW_LESSEQUAL) return 3;
    if (s == RW_GREATER) return 4;
    if (s == RW_NOTEQUAL) return 5;
    if (s == RW_GREATEREQUAL) return 6;
    if (s == RW_ALWAYS) return 7;
    syntaxError(context);
    return -1;
}

inline int toFillMode(Context& context)
{
    const auto& s = context.current.s;
    if (s == RW_WIREFRAME) return 0;
    if (s == RW_SOLID) return 1;
    syntaxError(context);
    return -1;
}

inline int toCullFace(Context& context)
{
    const auto& s = context.current.s;
    if (s == RW_NONE) return 0;
    if (s == RW_FRONT) return 1;
    if (s == RW_BACK) return 2;
    syntaxError(context);
    return -1;
}
