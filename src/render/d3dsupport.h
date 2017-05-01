#ifndef GAMEFRIENDS_D3DSUPPORT_H
#define GAMEFRIENDS_D3DSUPPORT_H

#include "../foundation/exception.h"
#include "../foundation/prerequest.h"
#include "d3dx12.h"
#include <d3d12.h>
#include <string>

GF_NAMESPACE_BEGIN

enum class PixelFormat;
enum class PrimitiveTopology;
enum class PixelBufferState;
enum class GpuCommandType;
enum class ComparisonFun;
enum class FillMode;
enum class CullingFace;
struct DepthState;
struct RasterizerState;
struct Viewport;

class Direct3DException : public Exception
{
public:
    explicit Direct3DException(const std::string& msg)
        : Exception(msg) {}
};

struct D3DMappings
{
    static const D3D12_CPU_DESCRIPTOR_HANDLE CPU_DESCRIPTOR_UNKOWN;
    static const D3D12_GPU_DESCRIPTOR_HANDLE GPU_DESCRIPTOR_UNKOWN;

    static DXGI_FORMAT DXGI_FORMAT_(PixelFormat pf);
    static D3D12_PRIMITIVE_TOPOLOGY PRIMITIVE_TOPOLOGY(PrimitiveTopology pt);
    static D3D12_RESOURCE_STATES RESOURCE_STATES(PixelBufferState state);
    static D3D12_RESOURCE_FLAGS RESOURCE_FLAGS(unsigned flags);
    static D3D12_COMMAND_LIST_TYPE COMMAND_LIST_TYPE(GpuCommandType type);
    static D3D12_COMPARISON_FUNC COMPARISON_FUNC(ComparisonFun fun);
    static D3D12_FILL_MODE FILL_MODE(FillMode m);
    static D3D12_CULL_MODE CULL_MODE(CullingFace f);
    static D3D12_DEPTH_STENCIL_DESC DEPTH_STENCIL_DESC(const DepthState& ds);
    static D3D12_RASTERIZER_DESC RASTERIZER_DESC(const RasterizerState& rs);
    static D3D12_VIEWPORT VIEWPORT(const Viewport& vp);
};

D3D12_PRIMITIVE_TOPOLOGY_TYPE correspondTopologyType(D3D12_PRIMITIVE_TOPOLOGY pt);

extern const D3D12_CPU_DESCRIPTOR_HANDLE CPU_DESCRIPTOR_UNKOWN;
extern const D3D12_GPU_DESCRIPTOR_HANDLE GPU_DESCRIPTOR_UNKOWN;

bool operator ==(D3D12_CPU_DESCRIPTOR_HANDLE a, D3D12_CPU_DESCRIPTOR_HANDLE b);
bool operator ==(D3D12_GPU_DESCRIPTOR_HANDLE a, D3D12_GPU_DESCRIPTOR_HANDLE b);
bool operator !=(D3D12_CPU_DESCRIPTOR_HANDLE a, D3D12_CPU_DESCRIPTOR_HANDLE b);
bool operator !=(D3D12_GPU_DESCRIPTOR_HANDLE a, D3D12_GPU_DESCRIPTOR_HANDLE b);

GF_NAMESPACE_END

#endif