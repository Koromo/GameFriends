#include "pixelbuffer.h"
#include "rendersystem.h"
#include "descriptorheap.h"
#include "linearallocator.h"
#include "d3dsupport.h"
#include "foundation/exception.h"

GF_NAMESPACE_BEGIN

PixelBuffer::PixelBuffer(const PixelBufferSetup& setup, const D3D12_CLEAR_VALUE* optimizedClear)
    : resource_()
    , srvFormat_(DXGI_FORMAT_UNKNOWN)
    , rtv_{}
    , dtv_{}
{
    const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto desc = CD3DX12_RESOURCE_DESC::Tex2D(D3DMappings::DXGI_FORMAT_(setup.baseFormat), setup.width,
        setup.height, static_cast<UINT16>(setup.arrayLength), static_cast<UINT16>(setup.mipLevels), 1, 0,
        D3DMappings::RESOURCE_FLAGS(setup.flags));
    const auto state = D3DMappings::RESOURCE_STATES(setup.state);

    ID3D12Resource* resource;
    if (FAILED(renderSystem.nativeDevice().CreateCommittedResource(
        &defaultHeap, D3D12_HEAP_FLAG_NONE, &desc, state, optimizedClear, IID_PPV_ARGS(&resource))))
    {
        /// LOG
        return;
    }

    resource->SetName(L"PixelBuffer");
    resource_ = makeComPtr(resource);

    if (!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
    {
        srvFormat_ = D3DMappings::DXGI_FORMAT_(setup.srvFormat);
    }

    rtv_.descriptor = CPU_DESCRIPTOR_UNKOWN;
    if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
    {
        rtv_.desc.Format = D3DMappings::DXGI_FORMAT_(setup.rtvFormat);
        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize == 1)
            {
                rtv_.desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            }
            else
            {
                rtv_.desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                rtv_.desc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
            }
        }

        const auto descriptor = DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)(1, false);
        rtv_.descriptor = descriptor.cpuAt;
        renderSystem.nativeDevice().CreateRenderTargetView(resource, &rtv_.desc, rtv_.descriptor);
    }

    dtv_.descriptor = CPU_DESCRIPTOR_UNKOWN;
    if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
    {
        dtv_.desc.Format = D3DMappings::DXGI_FORMAT_(setup.dtvFormat);
        if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            if (desc.DepthOrArraySize == 1)
            {
                dtv_.desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            }
            else
            {
                dtv_.desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                dtv_.desc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
            }
        }

        const auto descriptor = DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)(1, false);
        dtv_.descriptor = descriptor.cpuAt;
        renderSystem.nativeDevice().CreateDepthStencilView(resource, &dtv_.desc, dtv_.descriptor);
    }
}

PixelBuffer::PixelBuffer(const PixelBufferSetup& setup)
    : PixelBuffer(setup, nullptr)
{
}

PixelBuffer::PixelBuffer(const PixelBufferSetup& setup, const Color& optimizedClear)
    : PixelBuffer(setup, &CD3DX12_CLEAR_VALUE(D3DMappings::DXGI_FORMAT_(setup.rtvFormat), reinterpret_cast<const FLOAT*>(&optimizedClear)))
{
}

PixelBuffer::PixelBuffer(const PixelBufferSetup& setup, float optimizedDepth)
    : PixelBuffer(setup, &CD3DX12_CLEAR_VALUE(D3DMappings::DXGI_FORMAT_(setup.dtvFormat), optimizedDepth, 0))
{
}

PixelBuffer::PixelBuffer(ID3D12Resource* backBuffer, const D3D12_RENDER_TARGET_VIEW_DESC& view)
    : resource_(makeComPtr(backBuffer))
    , srvFormat_(DXGI_FORMAT_UNKNOWN)
    , rtv_{}
    , dtv_{}
{
    rtv_.desc = view;

    const auto descriptor = DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)(1, false);
    rtv_.descriptor = descriptor.cpuAt;
    renderSystem.nativeDevice().CreateRenderTargetView(backBuffer, &rtv_.desc, rtv_.descriptor);
}

void PixelBuffer::upload(ID3D12GraphicsCommandList& list, const PixelUpload& pixels)
{
    if (!resource_)
    {
        return;
    }

    const auto intermediateSize = static_cast<size_t>(GetRequiredIntermediateSize(resource_.get(), 0, 1));
    const auto intermediate = CpuAllocator()(intermediateSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

    D3D12_SUBRESOURCE_DATA srcData = {};
    srcData.pData = pixels.data;
    srcData.RowPitch = pixels.pixelSize * pixels.width;
    srcData.SlicePitch = srcData.RowPitch * pixels.height;

    const auto uploadedSize = UpdateSubresources<1>(&list, resource_.get(), intermediate.resource, intermediate.offset, 0, 1, &srcData);
    if (uploadedSize != intermediateSize)
    {
        /// LOG
    }
}

void PixelBuffer::createShaderResourceView(ID3D12Device& device, D3D12_CPU_DESCRIPTOR_HANDLE location)
{
    if (!resource_)
    {
        return;
    }

    const auto desc = resource_->GetDesc();
    check(!(desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE));

    D3D12_SHADER_RESOURCE_VIEW_DESC view = {};
    view.Format = srvFormat_;
    view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        if (desc.DepthOrArraySize == 1)
        {
            view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            view.Texture2D.MipLevels = desc.MipLevels;
        }
        else
        {
            view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray.ArraySize = desc.DepthOrArraySize;
            view.Texture2DArray.MipLevels = desc.MipLevels;
        }
    }

    device.CreateShaderResourceView(resource_.get(), &view, location);
}

RenderTargetView PixelBuffer::renderTargetView()
{
    return rtv_;
}

DepthTargetView PixelBuffer::depthTargetView()
{
    return dtv_;
}

ID3D12Resource* PixelBuffer::nativeResource()
{
    return resource_.get();
}

GF_NAMESPACE_END