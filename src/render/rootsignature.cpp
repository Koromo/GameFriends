#include "rootsignature.h"
#include "rendersystem.h"
#include "d3dsupport.h"
#include "../foundation/math.h"
#include <array>
#include <vector>

GF_NAMESPACE_BEGIN

RootSignatureObtain::RootSignatureObtain(const EachShaderSignature& sig)
    : eachSig_(sig)
{
}

ID3D12RootSignature& RootSignatureObtain::operator()() const
{
    return renderSystem.rootSignatures().obtain(eachSig_);
}

void RootSignatureCache::construct(ID3D12Device* device)
{
    device_ = device;
}

void RootSignatureCache::destruct()
{
    table_.clear();
    device_ = nullptr;
}

ID3D12RootSignature& RootSignatureCache::obtain(const EachShaderSignature& key)
{
    const auto hashCode = crc32(&key, sizeof(key));
    auto& obj = table_[hashCode];
    if (!obj)
    {
        const std::array<const ShaderSignature*, 3> priority = {
            &key.vs,
            &key.gs,
            &key.ps
        };
        const std::array<D3D12_SHADER_VISIBILITY, 3> visibility = {
            D3D12_SHADER_VISIBILITY_VERTEX,
            D3D12_SHADER_VISIBILITY_GEOMETRY,
            D3D12_SHADER_VISIBILITY_PIXEL
        };
        const std::array<D3D12_ROOT_SIGNATURE_FLAGS, 3> denyFlag = {
            D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS,
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS,
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
        };

        D3D12_ROOT_SIGNATURE_FLAGS flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

        std::vector<CD3DX12_ROOT_PARAMETER> rootParams;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;

        rootParams.reserve(NUM_HEAPS_PER_SHADER * priority.size());
        ranges.reserve(NUM_RANGES_PER_SHADER * priority.size());

        for (unsigned i = 0; i < priority.size(); ++i)
        {
            const auto sig = priority[i];

            size_t numBufferRanges = 0;
            size_t numSamplerRanges = 0;

            if (sig->cbv > 0)
            {
                ranges.emplace_back(
                    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, sig->cbv, 0));
                ++numBufferRanges;
            }

            if (sig->srv > 0)
            {
                ranges.emplace_back(
                    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, sig->srv, 0));
                ++numBufferRanges;
            }

            if (numBufferRanges > 0)
            {
                rootParams.emplace_back();
                CD3DX12_ROOT_PARAMETER::InitAsDescriptorTable(rootParams.back(),
                    numBufferRanges, ranges.data() + ranges.size() - numBufferRanges, visibility[i]);
            }

            if (sig->sampler > 0)
            {
                ranges.emplace_back(
                    CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, sig->sampler, 0));
                ++numSamplerRanges;
            }

            if (numSamplerRanges > 0)
            {
                rootParams.emplace_back();
                CD3DX12_ROOT_PARAMETER::InitAsDescriptorTable(rootParams.back(),
                    numSamplerRanges, ranges.data() + ranges.size() - numSamplerRanges, visibility[i]);
            }

            if (numBufferRanges + numSamplerRanges == 0)
            {
                flags |= denyFlag[i];
            }
        }

        const auto rsDesc = CD3DX12_ROOT_SIGNATURE_DESC(rootParams.size(), rootParams.data(), 0, nullptr, flags);
        ID3DBlob* rsBlob;
        ID3DBlob* err;

        const auto hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &err);
        if (FAILED(hr))
        {
            std::string msg = "Failed to serialize the root signature.";
            if (err)
            {
                msg += " ";
                msg += reinterpret_cast<char*>(err->GetBufferPointer());
                err->Release();
            }
            throw Direct3DException(msg);
        }

        ID3D12RootSignature* rs;
        verify<Direct3DException>(device_->CreateRootSignature(
            0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&rs)),
            "Failed to create the ID3D12RootSignature.");
        rs->SetName(L"RootSignature");

        obj = makeComPtr(rs);
    }

    return *obj;
}

GF_NAMESPACE_END