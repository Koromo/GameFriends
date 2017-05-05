#ifndef GAMEFRIENDS_ROOTSIGNATURE_H
#define GAMEFRIENDS_ROOTSIGNATURE_H

#include "../windowing/windowsinc.h"
#include "foundation/prerequest.h"
#include <d3d12.h>
#include <unordered_map>

GF_NAMESPACE_BEGIN

constexpr int NUM_HEAPS_PER_SHADER = 2; // CBV_SRV_UAV, SAMPLER
constexpr int NUM_RANGES_PER_SHADER = 3; // CBV, SRV, SAMPLER

struct ShaderSignature
{
    size_t cbv;
    size_t srv;
    size_t sampler;
};

struct EachShaderSignature
{
    ShaderSignature vs;
    ShaderSignature gs;
    ShaderSignature ps;
};

class RootSignatureObtain
{
private:
    EachShaderSignature eachSig_;

public:
    explicit RootSignatureObtain(const EachShaderSignature& sig);
    ID3D12RootSignature& operator ()() const;
};

class RootSignatureCache
{
private:
    ID3D12Device* device_;
    std::unordered_map<unsigned, ComPtr<ID3D12RootSignature>> table_;

public:
    void construct(ID3D12Device* device);
    void destruct();

    RootSignatureCache() = default;
    RootSignatureCache(const RootSignatureCache&) = delete;
    RootSignatureCache& operator =(const RootSignatureCache&) = delete;

    ID3D12RootSignature& obtain(const EachShaderSignature& key);
};

GF_NAMESPACE_END

#endif