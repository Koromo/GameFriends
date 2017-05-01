#include "gpucommand.h"
#include "shaderprogram.h"
#include "renderstate.h"
#include "drawcall.h"
#include "pixelbuffer.h"
#include "d3dsupport.h"
#include "../foundation/color.h"
#include "../foundation/exception.h"

GF_NAMESPACE_BEGIN

GpuCommands::GpuCommands(ID3D12CommandAllocator* allocator)
    : allocator_(makeComPtr(allocator))
{
}

void GpuCommands::reset()
{
    verify<Direct3DException>(allocator_->Reset(), "Failed to reset allocator");
}

ID3D12CommandAllocator& GpuCommands::nativeAllocator()
{
    return *allocator_;
}

GpuCommandBuilder::GpuCommandBuilder(ID3D12GraphicsCommandList* list)
    : list_(makeComPtr(list))
    , closed_(true)
    , hasCommands_(false)
{
}

bool GpuCommandBuilder::hasAnyCommands() const
{
    return hasCommands_;
}

void GpuCommandBuilder::record(GpuCommands& buildTarget)
{
    check(closed_);
    verify<Direct3DException>(list_->Reset(&buildTarget.nativeAllocator(), nullptr),
        "Failed to reset the command list");
    closed_ = false;
    hasCommands_ = false;
}

void GpuCommandBuilder::close()
{
    if (!closed_)
    {
        verify<Direct3DException>(list_->Close(),
            "Failed to close the command list");
        closed_ = true;
    }
}

void GpuCommandBuilder::clearRenderTarget(PixelBuffer& rt, const Color& clearColor)
{
    list_->ClearRenderTargetView(rt.renderTargetView().descriptor, reinterpret_cast<const float*>(&clearColor), 0, nullptr);
    hasCommands_ = true;
}

void GpuCommandBuilder::clearDepthTarget(PixelBuffer& ds, float depth)
{
    list_->ClearDepthStencilView(ds.depthTargetView().descriptor, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
    hasCommands_ = true;
}

void GpuCommandBuilder::setViewport(const Viewport& vp)
{
    const auto sr = CD3DX12_RECT(vp.left, vp.top, vp.left + vp.width, vp.top + vp.height);
    list_->RSSetViewports(1, &D3DMappings::VIEWPORT(vp));
    list_->RSSetScissorRects(1, &sr);
}

void GpuCommandBuilder::setShaderBindings(const ShaderBindings& bindings)
{
    const auto descriptorHeaps = bindings.usedDescriptorHeaps();
    const auto rootParams = bindings.rootParameters();

    const auto numDescriptorHeaps = descriptorHeaps.second - descriptorHeaps.first;
    const auto descriptorHeapHead = &(*descriptorHeaps.first);
    list_->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeapHead);

    const auto numRootParameters = rootParams.second - rootParams.first;
    auto rootParam = rootParams.first;
    for (int i = 0; i < numRootParameters; ++i)
    {
        list_->SetGraphicsRootDescriptorTable(i, *rootParam);
        ++rootParam;
    }
}

void GpuCommandBuilder::prepareDrawCall(const OptimizedDrawCall& drawCall)
{
    drawCall.prepare(*list_);
    hasCommands_ = true;
}

void GpuCommandBuilder::triggerDrawCall(const OptimizedDrawCall& drawCall)
{
    drawCall.trigger(*list_);
    hasCommands_ = true;
}

void GpuCommandBuilder::transition(PixelBuffer& resource, PixelBufferState befor, PixelBufferState after)
{
    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(&resource.nativeResource(),
        D3DMappings::RESOURCE_STATES(befor), D3DMappings::RESOURCE_STATES(after));
    list_->ResourceBarrier(1, &barrier);
    hasCommands_ = true;
}

void GpuCommandBuilder::uploadVertices(VertexData& vertexData)
{
    vertexData.upload(*list_);
    hasCommands_ = true;
}

void GpuCommandBuilder::drawableState(VertexData& vertexData)
{
    const auto b = vertexData.drawableState(*list_);
    hasCommands_ = (hasCommands_ || b);
}

void GpuCommandBuilder::uploadPixels(const PixelUpload& pixels, PixelBuffer& buffer)
{
    buffer.upload(*list_, pixels);
    hasCommands_ = true;
}

ID3D12GraphicsCommandList& GpuCommandBuilder::nativeList()
{
    return *list_;
}

void GpuCommandExecuter::construct(ID3D12Device* device, GpuCommandType type)
{
    device_ = device;
    type_ = D3DMappings::COMMAND_LIST_TYPE(type);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = type_;

    ID3D12CommandQueue* commandQueue;
    verify<Direct3DException>(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)),
        "Failed to create the ID3D12CommandQueue.");
    commandQueue->SetName(L"CommandQueue");
    commandQueue_ = makeComPtr(commandQueue);

    fenceEvent_.reset(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS));
    currentFenceValue_ = 1;

    ID3D12Fence* fence;
    verify<Direct3DException>(
        device_->CreateFence(currentFenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)),
        "Feiled to create the fence.");
    fence->SetName(L"Fence");
    fence_ = makeComPtr(fence);

    ID3D12CommandAllocator* allocator;
    verify<Direct3DException>(device->CreateCommandAllocator(type_, IID_PPV_ARGS(&allocator)),
        "Failed to create the ID3D12CommandAllocator");
    allocator->SetName(L"DefaultCommandAllocator");
    defaultAllocator_ = makeComPtr(allocator);
}

void GpuCommandExecuter::destruct()
{
    defaultAllocator_.reset();
    fence_.reset();
    fenceEvent_.reset();
    commandQueue_.reset();
    device_ = nullptr;
}

std::shared_ptr<GpuCommands> GpuCommandExecuter::createCommands()
{
    ID3D12CommandAllocator* allocator;
    verify<Direct3DException>(device_->CreateCommandAllocator(type_, IID_PPV_ARGS(&allocator)),
        "Failed to create the ID3D12CommandAllocator");
    allocator->SetName(L"CommandAllocator");

    return std::make_shared<GpuCommands>(allocator);
}

std::shared_ptr<GpuCommandBuilder> GpuCommandExecuter::createBuilder()
{
    ID3D12GraphicsCommandList* list;
    verify<Direct3DException>(device_->CreateCommandList(0, type_, defaultAllocator_.get(), nullptr, IID_PPV_ARGS(&list)),
        "Failed to create the ID3D12GraphicsCommandList.");
    list->SetName(L"CommandList");
    list->Close();

    return std::make_shared<GpuCommandBuilder>(list);
}

FenceValue GpuCommandExecuter::execute(GpuCommandBuilder& builder)
{
    builder.close();

    if (builder.hasAnyCommands())
    {
        const auto list = &builder.nativeList();
        commandQueue_->ExecuteCommandLists(1, CommandListCast(&list));
        ++currentFenceValue_;
        verify<Direct3DException>(commandQueue_->Signal(fence_.get(), currentFenceValue_), "Failed to Signal");
    }

    return currentFenceValue_;
}

bool GpuCommandExecuter::fenceCompleted(FenceValue val) const
{
    return val <= fence_->GetCompletedValue();
}

void GpuCommandExecuter::waitForFenceCompletion(FenceValue val)
{
    if (!fenceCompleted(val))
    {
        fence_->SetEventOnCompletion(val, fenceEvent_.get());
        WaitForSingleObject(fenceEvent_.get(), INFINITE);
    }
}

ID3D12CommandQueue& GpuCommandExecuter::nativeQueue()
{
    return *commandQueue_;
}

GF_NAMESPACE_END