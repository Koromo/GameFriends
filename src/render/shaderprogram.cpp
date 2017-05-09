#include "shaderprogram.h"
#include "pixelbuffer.h"
#include "rendersystem.h"
#include "../engine/logging.h"
#include "foundation/string.h"
#include "foundation/math.h"
#include "foundation/exception.h"
#include <utility>

GF_NAMESPACE_BEGIN

namespace
{
    size_t index(ShaderType t)
    {
        return static_cast<size_t>(t);
    }
}

ShaderParameters::ShaderParameters(ID3D12ShaderReflection* vs, ID3D12ShaderReflection* gs, ID3D12ShaderReflection* ps,
    const EachShaderSignature& signatures)
    : bindingMap_()
{
    struct ShaderInfo
    {
        ID3D12ShaderReflection* ref;
        const ShaderSignature* sig;
        BindingMap* bindings;
    };

    const std::array<ShaderInfo, 3> priority = {
        ShaderInfo{ vs, &signatures.vs, &bindingMap_[0] },
        ShaderInfo{ gs, &signatures.gs, &bindingMap_[1] },
        ShaderInfo{ ps, &signatures.ps, &bindingMap_[2] }
    };

    descriptorHeaps_.reserve(2);
    rootParameters_.reserve(NUM_HEAPS_PER_SHADER * priority.size());

    size_t sizeOfBufferHeap = 0;
    size_t sizeOfSamplerHeap = 0;
    for (unsigned i = 0; i < priority.size(); ++i)
    {
        const auto sig = priority[i].sig;
        sizeOfBufferHeap += (sig->cbv + sig->srv);
        sizeOfSamplerHeap += sig->sampler;
    }

    Descriptor bufferHeap = {};
    Descriptor samplerHeap = {};
    if (sizeOfBufferHeap > 0)
    {
        bufferHeap = DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)(sizeOfBufferHeap, true);
        descriptorHeaps_.emplace_back(bufferHeap.heap);
    }
    if (sizeOfSamplerHeap > 0)
    {
        samplerHeap = DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)(sizeOfSamplerHeap, true);
        descriptorHeaps_.emplace_back(samplerHeap.heap);
    }

    for (unsigned i = 0; i < priority.size(); ++i)
    {
        const auto ref = priority[i].ref;
        const auto sig = priority[i].sig;
        const auto map = priority[i].bindings;

        createBindingMap(ref, *sig, bufferHeap, samplerHeap, *map);

        if (sig->cbv + sig->srv > 0)
        {
            rootParameters_.emplace_back(bufferHeap.gpuAt);
            bufferHeap = bufferHeap.next(sig->cbv + sig->srv);
        }

        if (sig->sampler > 0)
        {
            rootParameters_.emplace_back(samplerHeap.gpuAt);
            samplerHeap = samplerHeap.next(sig->sampler);
        }
    }
}

void ShaderParameters::updateConstant(ShaderType type, const std::string& name, const void* data, size_t size)
{
    const auto found = bindingMap_[index(type)].var.find(name);
    if (found != std::cend(bindingMap_[index(type)].var))
    {
        const auto var = found->second;
        std::memcpy(var.ptr, data, size < var.size ? size : var.size);
    }
}

void ShaderParameters::updateShaderResource(ShaderType type, const std::string& name, PixelBuffer& resource)
{
    const auto found = bindingMap_[index(type)].sr.find(name);
    if (found != std::cend(bindingMap_[index(type)].sr))
    {
        const auto sr = found->second;
        const auto location = sr.location.cpuAt;
        resource.createShaderResourceView(renderSystem.nativeDevice(), location);
    }
}

void ShaderParameters::createBindingMap(ID3D12ShaderReflection* shader, const ShaderSignature& sig,
    Descriptor bufferHeap, Descriptor samplerHeap, BindingMap& out)
{
    out.cbuf.clear();
    out.var.clear();
    out.sr.clear();

    if (!shader)
    {
        return;
    }

    D3D12_SHADER_DESC shaderDesc = {};
    auto hr = shader->GetDesc(&shaderDesc);
    check(SUCCEEDED(hr));

    for (unsigned i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc;
        hr = shader->GetResourceBindingDesc(i, &bindDesc);
        check(SUCCEEDED(hr));

        switch (bindDesc.Type)
        {
        case D3D_SIT_CBUFFER: {
            ID3D12ShaderReflectionConstantBuffer* bufferRef = shader->GetConstantBufferByName(bindDesc.Name);
            check(!!bufferRef);

            D3D12_SHADER_BUFFER_DESC bufferDesc = {};
            hr = bufferRef->GetDesc(&bufferDesc);
            check(SUCCEEDED(hr));

            const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(ceiling<size_t>(bufferDesc.Size, 256));

            ID3D12Resource* resource;
            if (FAILED(renderSystem.nativeDevice().CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE,
                &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource))))
            {
                GF_LOG_WARN("Failed to create D3D12 constant buffer.");
                break;
            }
            resource->SetName(L"ConstantBuffer");

            ConstantBuffer bufferHolder;
            bufferHolder.resource = makeComPtr(resource);
            bufferHolder.location = bufferHeap.next(0 + bindDesc.BindPoint);

            D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
            view.BufferLocation = resource->GetGPUVirtualAddress();
            view.SizeInBytes = static_cast<UINT>(resourceDesc.Width);
            renderSystem.nativeDevice().CreateConstantBufferView(&view, bufferHolder.location.cpuAt);

            out.cbuf.emplace(bindDesc.Name, bufferHolder);

            char* mappedData;
            hr = resource->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
            check(SUCCEEDED(hr));

            for (UINT i = 0; i < bufferDesc.Variables; ++i)
            {
                const auto varRef = bufferRef->GetVariableByIndex(i);
                check(!!varRef);

                D3D12_SHADER_VARIABLE_DESC varDesc;
                hr = varRef->GetDesc(&varDesc);
                check(SUCCEEDED(hr));

                Variable var;
                var.ptr = mappedData + varDesc.StartOffset;
                var.size = varDesc.Size;

                if (varDesc.DefaultValue)
                {
                    std::memcpy(var.ptr, varDesc.DefaultValue, var.size);
                }

                out.var.emplace(varDesc.Name, var);
            }

            break;
        }

        case D3D_SIT_TEXTURE: {
            ShaderResource sr;
            sr.location = bufferHeap.next(sig.cbv + bindDesc.BindPoint);
            out.sr.emplace(bindDesc.Name, sr);
            break;
        }

        case D3D_SIT_SAMPLER:
            break;

        default:
            break;
        }
    }
}

HLSLShader::HLSLShader(const FilePath& path)
    : Resource(path)
    , model_()
    , entry_()
    , macros_()
    , compiledShaders_()
{
}

void HLSLShader::setModel(const std::string& model)
{
    model_ = model;
}

void HLSLShader::setEntry(const std::string& entry)
{
    entry_ = entry;
}

void HLSLShader::setMacros(std::vector<ShaderMacro>&& macros)
{
    macros_ = std::move(macros);
}

CompiledShader HLSLShader::compile()
{
    auto hashStr = entry_ + "-" + model_;
    for (const auto& m : macros_)
    {
        hashStr += "-" + m.name + "_" + m.value;
    }

    auto& compiled = compiledShaders_[hashStr];
    if (!compiled.code)
    {
#ifdef GF_DEBUG
        const auto FLAGS = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        const auto FLAGS = 0;
#endif

        std::vector<D3D_SHADER_MACRO> d3dMacros;
        d3dMacros.reserve(macros_.size() + 1);
        for (const auto& m : macros_)
        {
            d3dMacros.emplace_back(D3D_SHADER_MACRO{ m.name.c_str(), m.value.c_str() });
        }
        d3dMacros.emplace_back(D3D_SHADER_MACRO{ nullptr, nullptr });

        ID3DBlob* blob;
        ID3DBlob* err = NULL;

        const auto hr = D3DCompileFromFile(widen(path().os).c_str(), d3dMacros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entry_.c_str(), model_.c_str(), FLAGS, 0, &blob, &err);
        if (FAILED(hr))
        {
            if (err)
            {
                const std::string msg = static_cast<char*>(err->GetBufferPointer());
                err->Release();
                GF_LOG_ERROR("Shader compile error. {}", msg);
                throw ShaderCompileError(msg);
            }
            GF_LOG_ERROR("Shader compile error. File not found {}", path().os);
            throw ShaderCompileError("File not found (" + path().os + ").");
        }

        compiled.code = makeComPtr(blob);

        ID3D12ShaderReflection* ref;
        D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&ref));
        compiled.ref = makeComPtr(ref);

        check(ref);
    }

    CompiledShader ret;
    ret.code.pShaderBytecode = compiled.code->GetBufferPointer();
    ret.code.BytecodeLength = compiled.code->GetBufferSize();
    ret.reflect = compiled.ref.get();

    return ret;
}

bool HLSLShader::loadImpl()
{
    return true;
}

void HLSLShader::unloadImpl()
{
    compiledShaders_.clear();
}

ShaderProgram::ShaderProgram()
    : shaders_()
    , signatures_{}
{
    shaders_.fill(CompiledShader{});
}

void ShaderProgram::compile(ShaderType type, const std::string& path, const std::string& entry, std::vector<ShaderMacro>&& macros)
{
    std::string model;
    switch (type)
    {
    case ShaderType::vertex:
        model = "vs_5_0";
        break;

    case ShaderType::geometry:
        model = "gs_5_0";
        break;

    case ShaderType::pixel:
        model = "ps_5_0";
        break;

    default:
        check(false);
    }

    const auto shaderFile = resourceManager.template obtain<HLSLShader>(path);
    shaderFile->load();

    shaderFile->setEntry(entry);
    shaderFile->setModel(model);
    shaderFile->setMacros(std::move(macros));

    const auto shader = shaderFile->compile();
    shaders_[index(type)] = shader;
    
    const auto sig = signatureOf(*shader.reflect);
    switch (type)
    {
    case ShaderType::vertex:
        signatures_.vs = sig;
        break;

    case ShaderType::geometry:
        signatures_.gs = sig;
        break;

    case ShaderType::pixel:
        signatures_.ps = sig;
        break;

    default:
        check(false);
    }
}

std::shared_ptr<ShaderParameters> ShaderProgram::createParameters() const
{
    return std::make_shared<ShaderParameters>(
        shaders_[0].reflect, shaders_[1].reflect, shaders_[2].reflect, signatures_);
}

D3D12_SHADER_BYTECODE ShaderProgram::shaderStage(ShaderType type) const
{
    return shaders_[index(type)].code;
}

EachShaderSignature ShaderProgram::shaderSignatures() const
{
    return signatures_;
}

ShaderSignature ShaderProgram::signatureOf(ID3D12ShaderReflection& shader) const
{
    D3D12_SHADER_DESC desc = {};
    auto hr = shader.GetDesc(&desc);
    check(SUCCEEDED(hr));

    ShaderSignature sig = {};
    for (size_t i = 0; i < desc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bound;
        hr = shader.GetResourceBindingDesc(i, &bound);
        check(SUCCEEDED(hr));

        switch (bound.Type)
        {
        case D3D_SIT_CBUFFER:
            ++sig.cbv;
            break;

        case D3D_SIT_TEXTURE:
            ++sig.srv;
            break;

        case D3D_SIT_SAMPLER:
            ++sig.sampler;
            break;

        default:
            break;
        }
    }

    return sig;
}

GF_NAMESPACE_END