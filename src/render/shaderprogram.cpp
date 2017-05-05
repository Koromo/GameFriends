#include "shaderprogram.h"
#include "rootsignature.h"
#include "pixelbuffer.h"
#include "rendersystem.h"
#include "d3dsupport.h"
#include "foundation/string.h"
#include "foundation/math.h"

GF_NAMESPACE_BEGIN

namespace
{
    std::string shaderID(const std::string& path, const std::string& entry, const std::string& model,
        std::initializer_list<ShaderMacro>& macros)
    {
        std::string expands("-" + entry + "-" + model);
        for (const auto& m : macros)
        {
            expands += "-" + m.name + "_" + m.value;
        }

        auto p = path.find_last_of(".");
        if (p == std::string::npos)
        {
            p = path.length();
        }
        auto id = path;
        id.insert(p, expands);
        return id;
    }

    size_t index(ShaderType t)
    {
        return static_cast<size_t>(t);
    }
}

ShaderBindings::ShaderBindings(ID3D12ShaderReflection* vs, ID3D12ShaderReflection* gs, ID3D12ShaderReflection* ps,
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

void ShaderBindings::updateConstant(ShaderType type, const std::string& name, const void* data, size_t size)
{
    const auto found = bindingMap_[index(type)].var.find(name);
    if (found != std::cend(bindingMap_[index(type)].var))
    {
        const auto var = found->second;
        std::memcpy(var.ptr, data, size < var.size ? size : var.size);
    }
}

void ShaderBindings::updateShaderResource(ShaderType type, const std::string& name, PixelBuffer& resource)
{
    const auto found = bindingMap_[index(type)].sr.find(name);
    if (found != std::cend(bindingMap_[index(type)].sr))
    {
        const auto sr = found->second;
        const auto location = sr.location.cpuAt;
        resource.createShaderResourceView(renderSystem.nativeDevice(), location);
    }
}

void ShaderBindings::createBindingMap(ID3D12ShaderReflection* shader, const ShaderSignature& sig,
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
    verify<Direct3DException>(shader->GetDesc(&shaderDesc), "Failed to get the detail of shader.");

    for (unsigned i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc;
        shader->GetResourceBindingDesc(i, &bindDesc);

        switch (bindDesc.Type)
        {
        case D3D_SIT_CBUFFER: {
            ID3D12ShaderReflectionConstantBuffer* bufferRef = enforce<Direct3DException>(
                shader->GetConstantBufferByName(bindDesc.Name), "ref->GetConstantBufferByName(0)");

            D3D12_SHADER_BUFFER_DESC bufferDesc = {};
            verify<Direct3DException>(bufferRef->GetDesc(&bufferDesc), "cbRef->GetDesc(&cbDesc)");

            const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(ceiling<size_t>(bufferDesc.Size, 256));

            ID3D12Resource* resource;
            verify<Direct3DException>(renderSystem.nativeDevice().CreateCommittedResource(
                &uploadHeap, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)),
                "Failed to create the ID3D12Resource.");
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
            verify<Direct3DException>(
                resource->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)), "Failed to buffer mapping.");

            for (UINT i = 0; i < bufferDesc.Variables; ++i)
            {
                const auto varRef = bufferRef->GetVariableByIndex(i);

                D3D12_SHADER_VARIABLE_DESC varDesc;
                verify<Direct3DException>(varRef->GetDesc(&varDesc), "varRef->GetDesc(&varDesc)");

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

HLSLShader::HLSLShader(const std::string& id)
    : Resource(id)
{
}

void HLSLShader::loadImpl()
{
#ifdef GF_DEBUG
    const auto FLAGS = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    const auto FLAGS = 0;
#endif

    std::vector<D3D_SHADER_MACRO> d3dMacros;
    d3dMacros.reserve(macros.size() + 1);
    for (const auto& m: macros)
    {
        d3dMacros.emplace_back(D3D_SHADER_MACRO{ m.name.c_str(), m.value.c_str() });
    }
    d3dMacros.emplace_back(D3D_SHADER_MACRO{ nullptr, nullptr });

    ID3DBlob* blob;
    ID3DBlob* err = NULL;

    const auto hr = D3DCompileFromFile(widen(file).c_str(), d3dMacros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry.c_str(), model.c_str(), FLAGS, 0, &blob, &err);
    if (FAILED(hr))
    {
        if (err)
        {
            const std::string msg = static_cast<char*>(err->GetBufferPointer());
            err->Release();
            throw ShaderCompileError(msg);
        }
        throw FileException("File not found (" + file + ").");
    }
    code = makeComPtr(blob);

    byteCode.pShaderBytecode = blob->GetBufferPointer();
    byteCode.BytecodeLength = blob->GetBufferSize();

    ID3D12ShaderReflection* ref;
    verify<Direct3DException>(
        D3DReflect(code->GetBufferPointer(), code->GetBufferSize(), IID_PPV_ARGS(&ref)),
        "Failed to reflect the shader.");
    reflection = makeComPtr(ref);
}

void HLSLShader::unloadImpl()
{
    reflection.reset();
    code.reset();
}

ShaderProgram::ShaderProgram()
    : shaders_()
    , signatures_{}
{
}

void ShaderProgram::compile(ShaderType type, const std::string& path, const std::string& entry,
    std::initializer_list<ShaderMacro>& macros)
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

    const auto id = shaderID(path, entry, model, macros);
    const auto shader = resourceTable.template obtain<HLSLShader>(id);
    if (!shader->ready())
    {
        shader->file = path;
        shader->entry = entry;
        shader->model = model;
        shader->macros = macros;
        shader->load();
    }
    shaders_[index(type)] = shader;
    
    const auto sig = signatureOf(*shader->reflection);
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

std::shared_ptr<ShaderBindings> ShaderProgram::createBindings() const
{
    return std::make_shared<ShaderBindings>(
        shaders_[0].useable() ? shaders_[0]->reflection.get() : nullptr,
        shaders_[1].useable() ? shaders_[1]->reflection.get() : nullptr,
        shaders_[2].useable() ? shaders_[2]->reflection.get() : nullptr,
        signatures_
        );
}

ResourceInterface<const HLSLShader> ShaderProgram::shaderStage(ShaderType type) const
{
    return shaders_[index(type)];
}

ID3D12RootSignature& ShaderProgram::rootSignature() const
{
    return RootSignatureObtain(signatures_)();
}

ShaderSignature ShaderProgram::signatureOf(ID3D12ShaderReflection& shader) const
{
    D3D12_SHADER_DESC desc = {};
    verify<Direct3DException>(shader.GetDesc(&desc), "Failed to get the detail of shader.");

    ShaderSignature sig = {};
    for (size_t i = 0; i < desc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bound;
        shader.GetResourceBindingDesc(i, &bound);

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