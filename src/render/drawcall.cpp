#include "drawcall.h"
#include "shaderprogram.h"
#include "pixelbuffer.h"
#include "renderstate.h"
#include "pipeline.h"
#include "d3dsupport.h"

GF_NAMESPACE_BEGIN

OptimizedDrawCall::OptimizedDrawCall()
    : psoDesc_{}
    , renderTarget_(CPU_DESCRIPTOR_UNKOWN)
    , depthStencil_(CPU_DESCRIPTOR_UNKOWN)
    , viewport_{}
    , descriptorHeaps_()
    , rootParameters_()
    , vertices_{}
{
    psoDesc_.RasterizerState = D3DMappings::RASTERIZER_DESC(RasterizerState::DEFAULT);
    psoDesc_.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
    psoDesc_.DepthStencilState = D3DMappings::DEPTH_STENCIL_DESC(DepthState::DEFAULT);
    psoDesc_.SampleMask = UINT_MAX;
    psoDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    psoDesc_.SampleDesc.Count = 1;
}

void OptimizedDrawCall::setVertex(const VertexData& vertex, size_t vertexOffset, size_t vertexCount,
    size_t instanceOffset, size_t instanceCount)
{
    setVertexCommon(vertex, instanceOffset, instanceCount);
    vertices_.indexed = false;
    vertices_.vertOffset = vertexOffset;
    vertices_.vertCount = vertexCount;
}

void OptimizedDrawCall::setVertexIndexed(const VertexData& vertex, size_t vertexOffset, size_t indexOffset, size_t indexCount,
    size_t instanceOffset, size_t instanceCount)
{
    setVertexCommon(vertex, instanceOffset, instanceCount);
    vertices_.indexed = true;
    vertices_.vertOffset = vertexOffset;
    vertices_.indOffset = indexOffset;
    vertices_.indCount = indexCount;
    vertices_.indexBuffer = vertex.indexBuffer();
}

void OptimizedDrawCall::setShaderParameters(const ShaderParameters& param)
{
    descriptorHeaps_ = param.usedDescriptorHeaps();
    rootParameters_ = param.rootParameters();
}

void OptimizedDrawCall::setShaders(const ShaderProgram& shaders)
{
    psoDesc_.VS = shaders.shaderStage(ShaderType::vertex); 
    psoDesc_.GS = shaders.shaderStage(ShaderType::geometry); 
    psoDesc_.PS = shaders.shaderStage(ShaderType::pixel); 
    psoDesc_.pRootSignature = &shaders.rootSignature();
}

void OptimizedDrawCall::setDepthState(const DepthState& ds)
{
    psoDesc_.DepthStencilState = D3DMappings::DEPTH_STENCIL_DESC(ds);
}

void OptimizedDrawCall::setRasterizerState(const RasterizerState& rs)
{
    psoDesc_.RasterizerState = D3DMappings::RASTERIZER_DESC(rs);
}

void OptimizedDrawCall::setRenderTarget(PixelBuffer& rt)
{
    const auto view = rt.renderTargetView();
    renderTarget_ = view.descriptor;
    psoDesc_.NumRenderTargets = 1;
    psoDesc_.RTVFormats[0] = view.desc.Format;
}

void OptimizedDrawCall::setDepthTarget(PixelBuffer& dt)
{
    const auto view = dt.depthTargetView();
    depthStencil_ = view.descriptor;
    psoDesc_.DSVFormat = view.desc.Format;
}

void OptimizedDrawCall::setViewport(const Viewport& vp)
{
    viewport_ = D3DMappings::VIEWPORT(vp);
}

void OptimizedDrawCall::trigger(ID3D12GraphicsCommandList& list) const
{
    auto& pso = GraphicsPipelineStateObtain()(psoDesc_);
    list.SetPipelineState(&pso);
    list.SetGraphicsRootSignature(psoDesc_.pRootSignature);

    const auto pRTV = renderTarget_ != CPU_DESCRIPTOR_UNKOWN ? &renderTarget_ : nullptr;
    const auto pDSV = depthStencil_ != CPU_DESCRIPTOR_UNKOWN ? &depthStencil_ : nullptr;
    list.OMSetRenderTargets(!!pRTV, pRTV, FALSE, pDSV);

    const auto sr = CD3DX12_RECT(viewport_.TopLeftX, viewport_.TopLeftY,
        viewport_.TopLeftX + viewport_.Width, viewport_.TopLeftY + viewport_.Height);
    list.RSSetViewports(1, &viewport_);
    list.RSSetScissorRects(1, &sr);

    const auto numDescriptorHeaps = descriptorHeaps_.second - descriptorHeaps_.first;
    const auto descriptorHeapHead = &(*descriptorHeaps_.first);
    list.SetDescriptorHeaps(numDescriptorHeaps, descriptorHeapHead);

    const auto numRootParameters = rootParameters_.second - rootParameters_.first;
    auto rootParam = rootParameters_.first;
    for (int i = 0; i < numRootParameters; ++i)
    {
        list.SetGraphicsRootDescriptorTable(i, *rootParam);
        ++rootParam;
    }

    const auto numVertexBuffers = vertices_.vertexBuffers.second - vertices_.vertexBuffers.first;
    const auto vertexBuffers = &(*vertices_.vertexBuffers.first);
    list.IASetVertexBuffers(0, numVertexBuffers, vertexBuffers);
    list.IASetPrimitiveTopology(vertices_.primitiveTopology);

    if (vertices_.indexed)
    {
        list.IASetIndexBuffer(&vertices_.indexBuffer);
        list.DrawIndexedInstanced(vertices_.indCount, vertices_.instCount, vertices_.indOffset, vertices_.vertOffset, vertices_.instOffset);
    }
    else
    {
        list.DrawInstanced(vertices_.vertCount, vertices_.instCount, vertices_.vertOffset, vertices_.instOffset);
    }
}

void OptimizedDrawCall::setVertexCommon(const VertexData& vertex, size_t instanceOffset, size_t instanceCount)
{
    vertices_.instOffset = instanceOffset;
    vertices_.instCount = instanceCount;
    vertices_.primitiveTopology = vertex.primitiveTopology();
    vertices_.vertexBuffers = vertex.vertexBuffers();
    psoDesc_.InputLayout = vertex.inputLayout();
    psoDesc_.PrimitiveTopologyType = correspondTopologyType(vertices_.primitiveTopology);
}

GF_NAMESPACE_END