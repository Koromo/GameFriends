#ifndef GAMEFRIENDS_LINEARALLOCATOR_H
#define GAMEFRIENDS_LINEARALLOCATOR_H

#include "../windowing/windowsinc.h"
#include "foundation/prerequest.h"
#include <d3d12.h>
#include <vector>

GF_NAMESPACE_BEGIN

struct AllocatePoint
{
    size_t size; // aligned size
    void* data; // when upload allocation

    size_t offset;
    ID3D12Resource* resource;
};

class LinearAllocator
{
private:
    struct Page
    {
        void* mappedData;
        ComPtr<ID3D12Resource> resource;
        size_t offset;
    };

    ID3D12Device* device_;
    D3D12_HEAP_TYPE type_;
    std::vector<Page> pages_;

public:
    void construct(ID3D12Device* device, D3D12_HEAP_TYPE type);
    void destruct();

    LinearAllocator() = default;
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator =(const LinearAllocator&) = delete;

    AllocatePoint allocate(size_t size, size_t alignment) noexcept(false);

private:
    Page createNewPage();
};

struct CpuAllocator
{
    AllocatePoint operator ()(size_t size, size_t alignment) noexcept(false);
};

struct GpuAllocator
{
    AllocatePoint operator ()(size_t size, size_t alignment) noexcept(false);
};

GF_NAMESPACE_END

#endif
