#ifndef GAMEFRIENDS_SHADERPROGRAM_H
#define GAMEFRIENDS_SHADERPROGRAM_H

#include "descriptorheap.h"
#include "rootsignature.h"
#include "d3dsupport.h"
#include "../engine/resource.h"
#include "../engine/filesystem.h"
#include "../windowing/windowsinc.h"
#include "foundation/sortedvector.h"
#include "foundation/prerequest.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <memory>
#include <string>
#include <array>
#include <initializer_list>
#include <utility>
#include <unordered_map>
#include <cstring>

GF_NAMESPACE_BEGIN

class PixelBuffer;

enum class ShaderType
{
    vertex = 0,
    geometry = 1,
    pixel = 2
};

class ShaderCompileError : public Direct3DError
{
public:
    explicit ShaderCompileError(const std::string& msg)
        : Direct3DError(msg) {}
};

class ShaderParameters
{
private:
    struct ConstantBuffer
    {
        ComPtr<ID3D12Resource> resource;
        Descriptor location;
    };

    struct Variable
    {
        void* ptr;
        size_t size;
    };

    struct ShaderResource
    {
        Descriptor location;
    };

    struct BindingMap
    {
        std::unordered_map<std::string, ConstantBuffer> cbuf;
        std::unordered_map<std::string, Variable> var;
        std::unordered_map<std::string, ShaderResource> sr;
    };

    std::array<BindingMap, 3> bindingMap_;
    std::vector<ID3D12DescriptorHeap*> descriptorHeaps_;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> rootParameters_;

public:
    ShaderParameters(ID3D12ShaderReflection* vs, ID3D12ShaderReflection* gs, ID3D12ShaderReflection* ps,
        const EachShaderSignature& signatures);

    void updateConstant(ShaderType type, const std::string& name, const void* data, size_t size);
    void updateShaderResource(ShaderType type, const std::string& name, PixelBuffer& resource);

    auto usedDescriptorHeaps() const -> decltype(std::make_pair(std::cbegin(descriptorHeaps_), std::cend(descriptorHeaps_)))
    {
        return std::make_pair(std::cbegin(descriptorHeaps_), std::cend(descriptorHeaps_));
    }

    auto rootParameters() const -> decltype(std::make_pair(std::cbegin(rootParameters_), std::cend(rootParameters_)))
    {
        return std::make_pair(std::cbegin(rootParameters_), std::cend(rootParameters_));
    }

private:
    void createBindingMap(ID3D12ShaderReflection* shader, const ShaderSignature& sig,
        Descriptor buffersHead, Descriptor samplersHead, BindingMap& out);
};

struct ShaderMacro
{
    std::string name;
    std::string value;
};

struct CompiledShader
{
    D3D12_SHADER_BYTECODE code;
    ID3D12ShaderReflection* reflect;
};

class HLSLShader : public Resource
{
private:
    std::string model_;
    std::string entry_;

    struct MacroComp
    {
        bool operator()(const ShaderMacro& a, const ShaderMacro& b)
        {
            return a.name < b.name;
        }
    };
    std::vector<ShaderMacro> macros_; /// TODO: SortedVector

    struct ShaderHold
    {
        ComPtr<ID3DBlob> code;
        ComPtr<ID3D12ShaderReflection> ref;
    };
    std::unordered_map<std::string, ShaderHold> compiledShaders_;

public:
    HLSLShader(const EnginePath& path);

    void setModel(const std::string& model);
    void setEntry(const std::string& entry);
    void setMacros(std::vector<ShaderMacro>&& macros);
    CompiledShader compile() noexcept(false);

private:
    bool loadImpl();
    void unloadImpl();
};

class ShaderProgram
{
private:
    std::array<CompiledShader, 3> shaders_;
    EachShaderSignature signatures_;

public:
    ShaderProgram();

    void compile(ShaderType type, const std::string& path, const std::string& entry, std::vector<ShaderMacro>&& macros) noexcept(false);

    template <class Macros>
    void compile(ShaderType type, const std::string& path, const std::string& entry,
        Macros macroBegin, Macros macroEnd) noexcept(false)
    {
        compile(type, path, entry, std::vector<ShaderMacro>(macroBegin, macroEnd));
    }

    std::shared_ptr<ShaderParameters> createParameters() const  noexcept(false);

    D3D12_SHADER_BYTECODE shaderStage(ShaderType type) const;
    EachShaderSignature shaderSignatures() const;

private:
    ShaderSignature signatureOf(ID3D12ShaderReflection& shader) const;
};

GF_NAMESPACE_END

#endif