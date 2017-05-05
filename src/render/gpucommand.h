#ifndef GAMEFRIENDS_GPUCOMMAND_H
#define GAMEFRIENDS_GPUCOMMAND_H

#include "../windowing/windowsinc.h"
#include "foundation/prerequest.h"
#include <d3d12.h>
#include <type_traits>
#include <memory>

GF_NAMESPACE_BEGIN

struct Color;
class PixelBuffer;
class VertexData;
class OptimizedDrawCall;
struct PixelUpload;
enum class PixelBufferState;

using FenceValue = unsigned long long;

enum class GpuCommandType
{
    graphics = 0,
    copy = 1
};

class GpuCommands
{
private:
    ComPtr<ID3D12CommandAllocator> allocator_;

public:
    explicit GpuCommands(ID3D12CommandAllocator* allocator);
    void reset();

    ID3D12CommandAllocator& nativeAllocator();
};

class GpuCommandBuilder
{
private:
    ComPtr<ID3D12GraphicsCommandList> list_;
    bool closed_;
    bool hasCommands_;

public:
    explicit GpuCommandBuilder(ID3D12GraphicsCommandList* list);

    bool hasAnyCommands() const;
    void record(GpuCommands& buildTarget);
    void close();

    void clearRenderTarget(PixelBuffer& rt, const Color& clearColor);
    void clearDepthTarget(PixelBuffer& ds, float depth);

    void triggerDrawCall(const OptimizedDrawCall& drawCall);

    void transition(PixelBuffer& resource, PixelBufferState befor, PixelBufferState after);

    void uploadVertices(VertexData& vertexData);
    void drawableState(VertexData& vertexData);
    void uploadPixels(const PixelUpload& pixels, PixelBuffer& buffer);

    ID3D12GraphicsCommandList& nativeList();
};

class GpuCommandExecuter
{
private:
    ID3D12Device* device_;
    D3D12_COMMAND_LIST_TYPE type_;
    ComPtr<ID3D12CommandQueue> commandQueue_;

    struct HCloser
    {
        void operator ()(HANDLE h) { CloseHandle(h); }
    };

    ComPtr<ID3D12Fence> fence_;
    std::unique_ptr<std::remove_pointer_t<HANDLE>, HCloser> fenceEvent_;
    FenceValue currentFenceValue_;

    ComPtr<ID3D12CommandAllocator> defaultAllocator_;

public:
    void construct(ID3D12Device* device, GpuCommandType type);
    void destruct();

    GpuCommandExecuter() = default;
    GpuCommandExecuter(const GpuCommandExecuter&) = delete;
    GpuCommandExecuter& operator =(const GpuCommandExecuter&) = delete;

    std::shared_ptr<GpuCommands> createCommands();
    std::shared_ptr<GpuCommandBuilder> createBuilder();

    FenceValue execute(GpuCommandBuilder& builder);
    bool fenceCompleted(FenceValue val) const;
    void waitForFenceCompletion(FenceValue val);

    ID3D12CommandQueue& nativeQueue();
};

GF_NAMESPACE_END

#endif