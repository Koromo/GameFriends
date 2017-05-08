#include "pipeline.h"
#include "rendersystem.h"
#include "d3dsupport.h"
#include "../engine/logging.h"
#include "foundation/math.h"
#include "foundation/exception.h"

GF_NAMESPACE_BEGIN

ID3D12PipelineState& GraphicsPipelineStateObtain::operator ()(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
    return renderSystem.graphicsPipelineStates().obtain(desc);
}

void GraphicsPipelineStateCache::construct(ID3D12Device* device)
{
    device_ = device;
}

void GraphicsPipelineStateCache::destruct()
{
    table_.clear();
    device_ = nullptr;
}

ID3D12PipelineState& GraphicsPipelineStateCache::obtain(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& key)
{
    auto lowLevelDesc = key;
    lowLevelDesc.pRootSignature = nullptr;
    lowLevelDesc.InputLayout = {};
    lowLevelDesc.VS = {};
    lowLevelDesc.DS = {};
    lowLevelDesc.HS = {};
    lowLevelDesc.GS = {};
    lowLevelDesc.PS = {};

    const auto lowLevelHash = crc32(&lowLevelDesc, sizeof(lowLevelDesc));

    const auto hashing = [](const D3D12_SHADER_BYTECODE& s)
    {
        return crc32(s.pShaderBytecode, s.BytecodeLength);
    };
    const auto hashes = { lowLevelHash, hashing(key.VS), hashing(key.DS), hashing(key.HS), hashing(key.GS), hashing(key.PS) };
    const auto hashCode = hashCombine(std::cbegin(hashes), std::cend(hashes));

    auto& obj = table_[hashCode];

    if (!obj)
    {
        ID3D12PipelineState* pso;
        if (FAILED(device_->CreateGraphicsPipelineState(&key, IID_PPV_ARGS(&pso))))
        {
            GF_LOG_WARN("GraphicsPipelineState object creation error.");
            throw Direct3DException("Failed to create ID3D12PipelineState.");
        }
        pso->SetName(L"PipelineStateObject");

        obj = makeComPtr(pso);
    }

    return *obj;
}


GF_NAMESPACE_END