#ifndef GAMEFRIENDS_PIPELINESTATE_H
#define GAMEFRIENDS_PIPELINESTATE_H

#include "../windowing/windowsinc.h"
#include "foundation/prerequest.h"
#include <d3d12.h>
#include <unordered_map>

GF_NAMESPACE_BEGIN

struct GraphicsPipelineStateObtain
{
    ID3D12PipelineState& operator ()(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) noexcept(false);
};

struct GraphicsPipelineStateCache
{
private:
    ID3D12Device* device_;
    std::unordered_map<unsigned, ComPtr<ID3D12PipelineState>> table_;

public:
    void construct(ID3D12Device* device);
    void destruct();

    GraphicsPipelineStateCache() = default;
    GraphicsPipelineStateCache(const GraphicsPipelineStateCache&) = delete;
    GraphicsPipelineStateCache& operator=(const GraphicsPipelineStateCache&) = delete;

    ID3D12PipelineState& obtain(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& key) noexcept(false);
};

GF_NAMESPACE_END

#endif