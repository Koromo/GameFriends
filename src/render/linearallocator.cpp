#include "linearallocator.h"
#include "rendersystem.h"
#include "d3dsupport.h"
#include "../foundation/math.h"
#include "../foundation/exception.h"

GF_NAMESPACE_BEGIN

void LinearAllocator::construct(ID3D12Device* device, D3D12_HEAP_TYPE type)
{
    device_ = device;
    type_ = type;
}

void LinearAllocator::destruct()
{
    pages_.clear();
    device_ = nullptr;
}

AllocatePoint LinearAllocator::allocate(size_t size, size_t alignment)
{
    // align needs be a 2-power
    check(((alignment - 1) & alignment) == 0);

    if (pages_.empty())
    {
        pages_.emplace_back(createNewPage());
    }

    auto& page = pages_.back();
    auto desc = page.resource->GetDesc();
    auto alignedOffset = ceiling(page.offset, alignment);
    const auto alignedSize = size;// ceiling(size, alignment);

    if (alignedOffset + alignedSize > desc.Width)
    {
        pages_.emplace_back(createNewPage());
        page = pages_.back();
        desc = page.resource->GetDesc();
        alignedOffset = ceiling(page.offset, alignment);
    }

    check(alignedOffset + alignedSize <= desc.Width);
    page.offset = alignedOffset + alignedSize;

    AllocatePoint p = {};
    p.size = alignedSize;
    p.offset = alignedOffset;
    p.resource = page.resource.get();
    if (type_ == D3D12_HEAP_TYPE_UPLOAD)
    {
        p.data = reinterpret_cast<int8*>(page.mappedData) + alignedOffset;
    }

    return p;
}

LinearAllocator::Page LinearAllocator::createNewPage()
{
    const auto size = type_ == D3D12_HEAP_TYPE_UPLOAD ? 0x200000 : 0x10000; // 2MG:64k
    const auto flags = type_ == D3D12_HEAP_TYPE_UPLOAD ?
         D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    const auto heapProp = CD3DX12_HEAP_PROPERTIES(type_);
    const auto state = type_ == D3D12_HEAP_TYPE_UPLOAD ?
        D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

    const auto desc = CD3DX12_RESOURCE_DESC::Buffer(size, flags);

    ID3D12Resource* resource;
    verify<Direct3DException>(device_->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE,
        &desc, state, nullptr, IID_PPV_ARGS(&resource)),
        "Failed to create the ID3D12Resource.");
    resource->SetName(L"LinearBuffer");

    Page page;
    page.resource = makeComPtr(resource);
    page.offset = 0;
    if (type_ == D3D12_HEAP_TYPE_UPLOAD)
    {
        verify<Direct3DException>(resource->Map(0, nullptr, &page.mappedData),
            "failed to map.");
    }

    return page;
}

AllocatePoint CpuAllocator::operator ()(size_t size, size_t alignment)
{
    return renderSystem.linearAllocator(D3D12_HEAP_TYPE_UPLOAD).allocate(size, alignment);
}

AllocatePoint GpuAllocator::operator ()(size_t size, size_t alignment)
{
    return renderSystem.linearAllocator(D3D12_HEAP_TYPE_DEFAULT).allocate(size, alignment);
}

GF_NAMESPACE_END