#ifndef GAMEFRIENDS_PIXELBUFFER_H
#define GAMEFRIENDS_PIXELBUFFER_H

#include "../engine/pixelformat.h"
#include "../windowing/windowsinc.h"
#include "foundation/prerequest.h"
#include <d3d12.h>

GF_NAMESPACE_BEGIN

struct Color;

enum class PixelBufferState
{
    renderTarget,
    depthWrite,
    copyDest,
    present,
    genericRead
};

enum PixelBufferFlags
{
    PBF_None = 0,
    PBF_AllowRenderTarget = 0x1,
    PBF_AllowDepthTarget = 0x2,
    PBF_DenyShaderResource = 0x8,
};

struct PixelBufferSetup
{
    size_t width;
    size_t height;
    size_t arrayLength;
    size_t mipLevels;
    PixelFormat baseFormat;
    PixelFormat srvFormat;
    PixelFormat rtvFormat;
    PixelFormat dtvFormat;
    unsigned flags;
    PixelBufferState state;
};

struct PixelUpload
{
    const void* data;
    size_t pixelSize;
    size_t width;
    size_t height;
};

struct RenderTargetView
{
    D3D12_RENDER_TARGET_VIEW_DESC desc;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor;
};

struct DepthTargetView
{
    D3D12_DEPTH_STENCIL_VIEW_DESC desc;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor;
};

class PixelBuffer
{
private:
    ComPtr<ID3D12Resource> resource_;
    DXGI_FORMAT srvFormat_;
    RenderTargetView rtv_;
    DepthTargetView dtv_;

public:
    explicit PixelBuffer(const PixelBufferSetup& setup);
    PixelBuffer(const PixelBufferSetup& setup, const Color& optimizedClear);
    PixelBuffer(const PixelBufferSetup& setup, float optimizedDepth);
    PixelBuffer(ID3D12Resource* backBuffer, const D3D12_RENDER_TARGET_VIEW_DESC& view);

    void upload(ID3D12GraphicsCommandList& list, const PixelUpload& pixels);
    void createShaderResourceView(ID3D12Device& device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    RenderTargetView renderTargetView();
    DepthTargetView depthTargetView();
    ID3D12Resource& nativeResource();

private:
    PixelBuffer(const PixelBufferSetup& setup, const D3D12_CLEAR_VALUE* optimizedClear);
};

GF_NAMESPACE_END

#endif

