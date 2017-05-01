#ifndef GAMEFRIENDS_RENDERSYSTEM_H
#define GAMEFRIENDS_RENDERSYSTEM_H

#include "renderstate.h"
#include "gpucommand.h"
#include "linearallocator.h"
#include "descriptorheap.h"
#include "pipeline.h"
#include "rootsignature.h"
#include "../windowing/windowsinc.h"
#include "../foundation/prerequest.h"
#include <dxgi1_4.h>
#include <d3d12.h>
#include <memory>
#include <vector>
#include <array>

GF_NAMESPACE_BEGIN

class Window;
class PixelBuffer;

class RenderSystem
{
private:
    ComPtr<ID3D12Device> device_;
    ComPtr<IDXGISwapChain3> swapChain_;

    size_t frameIndex_;
    std::vector<std::shared_ptr<PixelBuffer>> backBuffers_;
    Viewport fullyViewport_;

    std::array<GpuCommandExecuter, 2> commandExecuters_;
    std::array<DescriptorHeap, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> descriptorHeaps_;
    std::array<LinearAllocator, 2> linearAllocators_;
    RootSignatureCache rootSignatures_;
    GraphicsPipelineStateCache graphicsPipelineStates_;

public:
    void startup(Window& chainWindow, size_t numBackBuffers);
    void shutdown();

    size_t numBackBuffers() const;
    size_t currentFrameIndex() const;
    PixelBuffer& backBuffer(size_t i);

    Viewport fullyViewport();
    GpuCommandExecuter& commandExecuter(GpuCommandType type);
    void present();

    DescriptorHeap&  descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
    LinearAllocator& linearAllocator(D3D12_HEAP_TYPE type);
    RootSignatureCache& rootSignatures();
    GraphicsPipelineStateCache& graphicsPipelineStates();

    ID3D12Device& nativeDevice();
};

extern RenderSystem renderSystem;

GF_NAMESPACE_END

#endif