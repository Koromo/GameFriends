#include "gpucommand.h"
#include "shaderprogram.h"
#include "renderstate.h"
#include "drawcall.h"
#include "pixelbuffer.h"
#include "d3dsupport.h"
#include "foundation/color.h"
#include "foundation/exception.h"
#include <string>

GF_NAMESPACE_BEGIN

GpuCommands::GpuCommands(ID3D12CommandAllocator* allocator)
    : allocator_(makeComPtr(allocator))
{
}

void GpuCommands::reset()
{
    if (FAILED(allocator_->Reset()))
    {
        /// LOG
    }
}

ID3D12CommandAllocator& GpuCommands::nativeAllocator()
{
    return *allocator_;
}

GpuCommandBuilder::GpuCommandBuilder(ID3D12GraphicsCommandList* list)
    : list_(makeComPtr(list))
{
}

void GpuCommandBuilder::record(GpuCommands& buildTarget)
{
    if (FAILED(list_->Reset(&buildTarget.nativeAllocator(), nullptr)))
    {
        /// LOG:
    }
}

void GpuCommandBuilder::close()
{
    if (FAILED(list_->Close()))
    {
        /// LOG
    }
}

void GpuCommandBuilder::clearRenderTarget(PixelBuffer& rt, const Color& clearColor)
{
    list_->ClearRenderTargetView(rt.renderTargetView().descriptor, reinterpret_cast<const float*>(&clearColor), 0, nullptr);
}

void GpuCommandBuilder::clearDepthTarget(PixelBuffer& ds, float depth)
{
    list_->ClearDepthStencilView(ds.depthTargetView().descriptor, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void GpuCommandBuilder::triggerDrawCall(const OptimizedDrawCall& drawCall)
{
    drawCall.trigger(*list_);
}

void GpuCommandBuilder::transition(PixelBuffer& resource, PixelBufferState befor, PixelBufferState after)
{
    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.nativeResource(),
        D3DMappings::RESOURCE_STATES(befor), D3DMappings::RESOURCE_STATES(after));
    list_->ResourceBarrier(1, &barrier);
}

void GpuCommandBuilder::uploadVertices(VertexData& vertexData)
{
    vertexData.upload(*list_);
}

void GpuCommandBuilder::drawableState(VertexData& vertexData)
{
    vertexData.drawableState(*list_);
}

void GpuCommandBuilder::uploadPixels(const PixelUpload& pixels, PixelBuffer& buffer)
{
    buffer.upload(*list_, pixels);
}

ID3D12GraphicsCommandList& GpuCommandBuilder::nativeList()
{
    return *list_;
}

void GpuCommandExecuter::construct(ID3D12Device* device, GpuCommandType type)
{
    device_ = device;
    type_ = D3DMappings::COMMAND_LIST_TYPE(type);
    const auto typeNumber = std::to_string(type_);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = type_;

    ID3D12CommandQueue* commandQueue;
    if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue))))
    {
        /// LOG
        throw Direct3DError("ID3D12CommandQueue (type " + typeNumber + ") creation failed.");
    }
    commandQueue->SetName(L"CommandQueue");
    commandQueue_ = makeComPtr(commandQueue);

    fenceEvent_.reset(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS));
    currentFenceValue_ = 1;

    ID3D12Fence* fence;
    if (FAILED(device_->CreateFence(currentFenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    {
        /// LOG
        throw Direct3DError("ID3D12Fence creation failed.");
    }
    fence->SetName(L"Fence");
    fence_ = makeComPtr(fence);

    ID3D12CommandAllocator* allocator;
    if (FAILED(device->CreateCommandAllocator(type_, IID_PPV_ARGS(&allocator))))
    {
        /// LOG
        throw Direct3DError("GpuCommandExecuter initialization failed.");
    }
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
    if (FAILED(device_->CreateCommandAllocator(type_, IID_PPV_ARGS(&allocator))))
    {
        /// LOG:
        throw Direct3DException("Failed to create the ID3D12CommandAllocator.");
    }
    allocator->SetName(L"CommandAllocator");

    return std::make_shared<GpuCommands>(allocator);
}

std::shared_ptr<GpuCommandBuilder> GpuCommandExecuter::createBuilder()
{
    ID3D12GraphicsCommandList* list;
    if (FAILED(device_->CreateCommandList(0, type_, defaultAllocator_.get(), nullptr, IID_PPV_ARGS(&list))))
    {
        /// LOG
        throw Direct3DException("Failed to create the ID3D12GraphicsCommandList.");
    }
    list->SetName(L"CommandList");
    list->Close();

    return std::make_shared<GpuCommandBuilder>(list);
}

FenceValue GpuCommandExecuter::execute(GpuCommandBuilder& builder)
{
    const auto list = &builder.nativeList();
    commandQueue_->ExecuteCommandLists(1, CommandListCast(&list));
    ++currentFenceValue_;

    if (FAILED(commandQueue_->Signal(fence_.get(), currentFenceValue_)))
    {
        /// LOG
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