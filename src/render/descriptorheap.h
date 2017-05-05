#ifndef GAMEFRIENDS_DESCRIPTORHEAP_H
#define GAMEFRIENDS_DESCRIPTORHEAP_H

#include "../windowing/windowsinc.h"
#include "foundation/prerequest.h"
#include <d3d12.h>
#include <vector>

GF_NAMESPACE_BEGIN

struct Descriptor
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpuAt;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuAt;
    size_t stride;
    ID3D12DescriptorHeap* heap;

    Descriptor next(size_t i) const;
};

class DescriptorHeap
{
private:
    struct Page
    {
        ComPtr<ID3D12DescriptorHeap> heap;
        size_t offset;
    };

    ID3D12Device* device_;
    D3D12_DESCRIPTOR_HEAP_TYPE type_;
    size_t stride_;
    std::vector<Page> pages_;
    std::vector<Page> shaderPages_;

public:
    void construct(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type);
    void destruct();

    DescriptorHeap() = default;
    DescriptorHeap(const DescriptorHeap&) = delete;
    DescriptorHeap& operator =(const DescriptorHeap&) = delete;

    Descriptor allocate(size_t size, bool shaderVisible);

private:
    Page createNewPage(bool shaderVisivle);
};

struct DescriptorAllocator
{
    D3D12_DESCRIPTOR_HEAP_TYPE type;
    explicit DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type_);
    Descriptor operator ()(size_t size, bool shaderVisible);
};

GF_NAMESPACE_END

#endif