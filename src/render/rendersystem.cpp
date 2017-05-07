#include "rendersystem.h"
#include "pixelbuffer.h"
#include "d3dsupport.h"
#include "../windowing/window.h"
#include "../engine/pixelformat.h"
#include "foundation/color.h"

GF_NAMESPACE_BEGIN

namespace
{
    const auto RENDER_TARGET_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
}

void RenderSystem::startup(Window& chainWindow, size_t numBackBuffers)
{
#ifdef GF_DEBUG
    ID3D12Debug* debug;
    if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        /// LOG:
    }
    else
    {
        debug->EnableDebugLayer();
        debug->Release();
    }
#endif

    numBackBuffers_ = numBackBuffers;

    ID3D12Device* device;
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device))))
    {
        /// LOG
        throw Direct3DError("ID3D12Device creation failed");
    }
    device->SetName(L"Device");
    device_ = makeComPtr(device);

    commandExecuter(GpuCommandType::graphics).construct(device, GpuCommandType::graphics);
    commandExecuter(GpuCommandType::copy).construct(device, GpuCommandType::copy);

    IDXGIFactory4* factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    {
        /// LOG
        throw Direct3DError("IDXGIFactory creation failed.");
    }
    GF_SCOPE_EXIT{ factory->Release(); };

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = chainWindow.clientWidth();
    swapChainDesc.Height = chainWindow.clientHeight();
    swapChainDesc.Format = RENDER_TARGET_FORMAT;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = numBackBuffers;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    IDXGISwapChain1* swapChain1;
    if (FAILED(factory->CreateSwapChainForHwnd(&commandExecuter(GpuCommandType::graphics).nativeQueue(),
        chainWindow.handle(), &swapChainDesc, nullptr, nullptr, &swapChain1)))
    {
        /// LOG
        throw Direct3DError("IDXGISwapChain1 creation failed.");
    }
    GF_SCOPE_EXIT{ swapChain1->Release(); };

    IDXGISwapChain3* swapChain3;
    if (FAILED(swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain3))))
    {
        /// LOG
        throw Direct3DError("IDXGISwapChain3 query failed.");
    }
    swapChain_ = makeComPtr(swapChain3);

    descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].construct(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].construct(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].construct(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].construct(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    linearAllocator(D3D12_HEAP_TYPE_DEFAULT).construct(device, D3D12_HEAP_TYPE_DEFAULT);
    linearAllocator(D3D12_HEAP_TYPE_UPLOAD).construct(device, D3D12_HEAP_TYPE_UPLOAD);

    rootSignatures_.construct(device);
    graphicsPipelineStates_.construct(device);

    backBuffers_.resize(numBackBuffers);
    for (UINT i = 0; i < numBackBuffers; ++i)
    {
        ID3D12Resource* backBuffer;
        if (FAILED(swapChain3->GetBuffer(i, IID_PPV_ARGS(&backBuffer))))
        {
            /// LOG
            throw Direct3DError("Getting back buffers failed.");
        }
        backBuffer->SetName(L"BackBuffer");

        D3D12_RENDER_TARGET_VIEW_DESC view = {};
        view.Format = swapChainDesc.Format;
        view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        backBuffers_[i] = std::make_shared<PixelBuffer>(backBuffer, view);
    }
    frameIndex_ = swapChain_->GetCurrentBackBufferIndex();

    fullyViewport_.left = fullyViewport_.top = 0;
    fullyViewport_.maxDepth = 1;
    fullyViewport_.minDepth = 0;
    fullyViewport_.width = static_cast<float>(chainWindow.clientWidth());
    fullyViewport_.height = static_cast<float>(chainWindow.clientHeight());

    /// LOG
}

void RenderSystem::shutdown()
{
    backBuffers_.clear();

    graphicsPipelineStates_.destruct();
    rootSignatures_.destruct();

    linearAllocator(D3D12_HEAP_TYPE_DEFAULT).destruct();
    linearAllocator(D3D12_HEAP_TYPE_UPLOAD).destruct();

    descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).destruct();
    descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).destruct();
    descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).destruct();
    descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV).destruct();

    swapChain_.reset();

    commandExecuter(GpuCommandType::graphics).destruct();
    commandExecuter(GpuCommandType::copy).destruct();

    device_.reset();

    /// LOG
}

size_t RenderSystem::numBackBuffers() const
{
    return numBackBuffers_;
}

size_t RenderSystem::currentFrameIndex() const
{
    return frameIndex_;
}

PixelBuffer& RenderSystem::backBuffer(size_t i)
{
    return *backBuffers_[i];
}

Viewport RenderSystem::fullyViewport()
{
    return fullyViewport_;
}

GpuCommandExecuter& RenderSystem::commandExecuter(GpuCommandType type)
{
    return commandExecuters_[static_cast<size_t>(type)];
}

void RenderSystem::present()
{
    if (FAILED(swapChain_->Present(1, 0)))
    {
        /// LOG:
    }
    frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
}

DescriptorHeap& RenderSystem::descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    return descriptorHeaps_[type];
}

LinearAllocator& RenderSystem::linearAllocator(D3D12_HEAP_TYPE type)
{
    return linearAllocators_[type - 1];
}

RootSignatureCache& RenderSystem::rootSignatures()
{
    return rootSignatures_;
}

GraphicsPipelineStateCache& RenderSystem::graphicsPipelineStates()
{
    return graphicsPipelineStates_;
}

ID3D12Device& RenderSystem::nativeDevice()
{
    return *device_;
}

RenderSystem renderSystem;

GF_NAMESPACE_END