#ifndef GAMEFRIENDS_DRAWCALL_H
#define GAMEFRIENDS_DRAWCALL_H

#include "shaderprogram.h"
#include "vertexdata.h"
#include "../foundation/prerequest.h"
#include <d3d12.h>

GF_NAMESPACE_BEGIN

class PixelBuffer;
struct DepthState;
struct RasterizerState;
struct Viewport;

class OptimizedDrawCall
{
private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_;
    D3D12_CPU_DESCRIPTOR_HANDLE renderTarget_;
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencil_;
    D3D12_VIEWPORT viewport_;
    decltype(std::declval<ShaderParameters>().usedDescriptorHeaps()) descriptorHeaps_;
    decltype(std::declval<ShaderParameters>().rootParameters()) rootParameters_;

    struct Vertices
    {
        bool indexed;

        size_t indOffset;
        size_t indCount;

        size_t vertOffset;
        size_t vertCount;

        size_t instOffset;
        size_t instCount;

        decltype(std::declval<VertexData>().vertexBuffers()) vertexBuffers;
        D3D12_INDEX_BUFFER_VIEW indexBuffer;
        D3D12_PRIMITIVE_TOPOLOGY primitiveTopology;
    } vertices_;

public:
    OptimizedDrawCall();

    void setVertex(const VertexData& vertex, size_t vertexOffset, size_t vertexCount,
        size_t instanceOffset, size_t instanceCount);
    void setVertexIndexed(const VertexData& vertex, size_t vertexOffset, size_t indexOffset, size_t indexCount,
        size_t instanceOffset, size_t instanceCount);

    void setShaderParameters(const ShaderParameters& param);
    void setShaders(const ShaderProgram& shaders);
    void setDepthState(const DepthState& ds);
    void setRasterizerState(const RasterizerState& rs);
    void setRenderTarget(PixelBuffer& rt);
    void setDepthTarget(PixelBuffer& dt);
    void setViewport(const Viewport& vp);

    void prepare(ID3D12GraphicsCommandList& list) const;
    void trigger(ID3D12GraphicsCommandList& list) const;

private:
    void setVertexCommon(const VertexData& vertex, size_t instanceOffset, size_t instanceCount);
};

GF_NAMESPACE_END

#endif