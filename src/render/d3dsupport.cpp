#include "d3dsupport.h"
#include "gpucommand.h"
#include "vertexdata.h"
#include "pixelbuffer.h"
#include "renderstate.h"
#include "../engine/pixelformat.h"

GF_NAMESPACE_BEGIN

DXGI_FORMAT D3DMappings::DXGI_FORMAT_(PixelFormat pf)
{
    switch (pf)
    {
    case PixelFormat::RGBA8_uint:  return DXGI_FORMAT_R8G8B8A8_UINT;
    case PixelFormat::RGBA8_unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case PixelFormat::R16:       return DXGI_FORMAT_R16_TYPELESS;
    case PixelFormat::R16_float: return DXGI_FORMAT_R16_FLOAT;
    case PixelFormat::R32:       return DXGI_FORMAT_R32_TYPELESS;
    case PixelFormat::R32_float: return DXGI_FORMAT_R32_FLOAT;
    case PixelFormat::RG16:       return DXGI_FORMAT_R16G16_TYPELESS;
    case PixelFormat::RG16_float: return DXGI_FORMAT_R16G16_FLOAT;
    case PixelFormat::RG32:       return DXGI_FORMAT_R32G32_TYPELESS;
    case PixelFormat::RG32_float: return DXGI_FORMAT_R32G32_FLOAT;
    case PixelFormat::RGB32_float: return DXGI_FORMAT_R32G32B32_FLOAT;
    case PixelFormat::RGBA32_float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case PixelFormat::D16_unorm: return DXGI_FORMAT_D16_UNORM;
    case PixelFormat::D32_float: return DXGI_FORMAT_D32_FLOAT;
    default: check(false); return DXGI_FORMAT_UNKNOWN;
    }
}

D3D12_PRIMITIVE_TOPOLOGY D3DMappings::PRIMITIVE_TOPOLOGY(PrimitiveTopology pt)
{
    switch (pt)
    {
    case PrimitiveTopology::lines: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case PrimitiveTopology::lineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case PrimitiveTopology::triangles: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PrimitiveTopology::triangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    default: check(false); return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    }
}

D3D12_RESOURCE_STATES D3DMappings::RESOURCE_STATES(PixelBufferState state)
{
    switch (state)
    {
    case PixelBufferState::renderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case PixelBufferState::depthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case PixelBufferState::copyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
    case PixelBufferState::present: return D3D12_RESOURCE_STATE_PRESENT;
    case PixelBufferState::genericRead: return D3D12_RESOURCE_STATE_GENERIC_READ;
    default: check(false); return D3D12_RESOURCE_STATE_COMMON;
    }
}

D3D12_RESOURCE_FLAGS D3DMappings::RESOURCE_FLAGS(unsigned flags)
{
    D3D12_RESOURCE_FLAGS d3dFlags = D3D12_RESOURCE_FLAG_NONE;
    if (flags & PBF_AllowRenderTarget)
    {
        d3dFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    if (flags & PBF_AllowDepthTarget)
    {
        d3dFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    if (flags & PBF_DenyShaderResource)
    {
        d3dFlags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }
    return d3dFlags;
}

D3D12_COMMAND_LIST_TYPE D3DMappings::COMMAND_LIST_TYPE(GpuCommandType type)
{
    switch (type)
    {
    case GpuCommandType::graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case GpuCommandType::copy: return D3D12_COMMAND_LIST_TYPE_COPY;
    default:
        check(false);
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
}

D3D12_COMPARISON_FUNC D3DMappings::COMPARISON_FUNC(ComparisonFun fun)
{
    switch (fun)
    {
    case ComparisonFun::never: return D3D12_COMPARISON_FUNC_NEVER;
    case ComparisonFun::less: return D3D12_COMPARISON_FUNC_LESS;
    case ComparisonFun::equal: return D3D12_COMPARISON_FUNC_EQUAL;
    case ComparisonFun::lessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case ComparisonFun::greater: return D3D12_COMPARISON_FUNC_GREATER;
    case ComparisonFun::notEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case ComparisonFun::greaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case ComparisonFun::always: return D3D12_COMPARISON_FUNC_ALWAYS;
    default: check(false); return D3D12_COMPARISON_FUNC_ALWAYS;
    }
}

D3D12_FILL_MODE D3DMappings::FILL_MODE(FillMode m)
{
    switch (m)
    {
    case FillMode::wireframe: return D3D12_FILL_MODE_WIREFRAME;
    case FillMode::solid: return D3D12_FILL_MODE_SOLID;
    default: check(false); return D3D12_FILL_MODE_SOLID;
    }
}

D3D12_CULL_MODE D3DMappings::CULL_MODE(CullingFace f)
{
    switch (f)
    {
    case CullingFace::none: return D3D12_CULL_MODE_NONE;
    case CullingFace::front: return D3D12_CULL_MODE_FRONT;
    case CullingFace::back: return D3D12_CULL_MODE_BACK;
    default: check(false); return D3D12_CULL_MODE_NONE;
    }
}

D3D12_DEPTH_STENCIL_DESC D3DMappings::DEPTH_STENCIL_DESC(const DepthState& ds)
{
    D3D12_DEPTH_STENCIL_DESC d = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
    d.DepthEnable = ds.depthEnable;
    d.DepthFunc = D3DMappings::COMPARISON_FUNC(ds.depthFun);
    return d;
}

D3D12_RASTERIZER_DESC D3DMappings::RASTERIZER_DESC(const RasterizerState& rs)
{
    D3D12_RASTERIZER_DESC r = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
    r.FillMode = D3DMappings::FILL_MODE(rs.fillMode);
    r.CullMode = D3DMappings::CULL_MODE(rs.cullFace);
    r.DepthClipEnable = rs.depthClip;
    return r;
}

D3D12_VIEWPORT D3DMappings::VIEWPORT(const Viewport& vp)
{
    D3D12_VIEWPORT v = {};
    v.TopLeftX = vp.left;
    v.TopLeftY = vp.top;
    v.Width = vp.width;
    v.Height = vp.height;
    v.MinDepth = vp.minDepth;
    v.MaxDepth = vp.maxDepth;
    return v;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE correspondTopologyType(D3D12_PRIMITIVE_TOPOLOGY pt)
{
    switch (pt)
    {
    case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    default:
        check(false);
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}

const D3D12_CPU_DESCRIPTOR_HANDLE CPU_DESCRIPTOR_UNKOWN = { static_cast<SIZE_T>(0 - 1) };
const D3D12_GPU_DESCRIPTOR_HANDLE GPU_DESCRIPTOR_UNKOWN = { static_cast<UINT64>(0 - 1) };

bool operator ==(D3D12_CPU_DESCRIPTOR_HANDLE a, D3D12_CPU_DESCRIPTOR_HANDLE b)
{
    return a.ptr == b.ptr;
}

bool operator ==(D3D12_GPU_DESCRIPTOR_HANDLE a, D3D12_GPU_DESCRIPTOR_HANDLE b)
{
    return a.ptr == b.ptr;
}

bool operator !=(D3D12_CPU_DESCRIPTOR_HANDLE a, D3D12_CPU_DESCRIPTOR_HANDLE b)
{
    return !(a == b);
}

bool operator !=(D3D12_GPU_DESCRIPTOR_HANDLE a, D3D12_GPU_DESCRIPTOR_HANDLE b)
{
    return !(a == b);
}

GF_NAMESPACE_END