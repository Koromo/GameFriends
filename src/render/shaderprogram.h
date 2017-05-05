#ifndef GAMEFRIENDS_SHADERPROGRAM_H
#define GAMEFRIENDS_SHADERPROGRAM_H

#include "descriptorheap.h"
#include "rootsignature.h"
#include "../engine/resource.h"
#include "../windowing/windowsinc.h"
#include "../foundation/sortedvector.h"
#include "../foundation/exception.h"
#include "../foundation/prerequest.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <memory>
#include <string>
#include <array>
#include <initializer_list>
#include <utility>
#include <cstring>

GF_NAMESPACE_BEGIN

class PixelBuffer;

enum class ShaderType
{
    vertex = 0,
    geometry = 1,
    pixel = 2
};

class ShaderCompileError : public Exception
{
public:
    explicit ShaderCompileError(const std::string& msg)
        : Exception(msg) {}
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

class HLSLShader : public Resource
{
public:
    std::string file;
    std::string model;
    std::string entry;

    struct MacroComp
    {
        bool operator()(const ShaderMacro& a, const ShaderMacro& b)
        {
            return a.name < b.name;
        }
    };
    std::vector<ShaderMacro> macros; /// TODO: SortedVector

    ComPtr<ID3DBlob> code;
    D3D12_SHADER_BYTECODE byteCode;
    ComPtr<ID3D12ShaderReflection> reflection;

    HLSLShader(const std::string& id);

private:
    void loadImpl();
    void unloadImpl();
};

class ShaderProgram
{
private:
    std::array<ResourceInterface<HLSLShader>, 3> shaders_;
    EachShaderSignature signatures_;

public:
    ShaderProgram();

    void compile(ShaderType type, const std::string& path, const std::string& entry,
        std::initializer_list<ShaderMacro>& macros = std::initializer_list<ShaderMacro>());
    std::shared_ptr<ShaderParameters> createParameters() const;

    ResourceInterface<const HLSLShader> shaderStage(ShaderType type) const;
    ID3D12RootSignature& rootSignature() const;

private:
    ShaderSignature signatureOf(ID3D12ShaderReflection& shader) const;
};

GF_NAMESPACE_END

#endif