#include "descriptorheap.h"
#include "rendersystem.h"
#include "d3dsupport.h"
#include "../engine/logging.h"

GF_NAMESPACE_BEGIN

Descriptor Descriptor::next(size_t i) const
{
    Descriptor n = *this;
    n.cpuAt.ptr += stride * i;
    n.gpuAt.ptr += stride * i;
    return n;
}

void DescriptorHeap::construct(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    device_ = device;
    type_ = type;
    stride_ = device->GetDescriptorHandleIncrementSize(type);
}

void DescriptorHeap::destruct()
{
    pages_.clear();
    shaderPages_.clear();
    device_ = nullptr;
}

Descriptor DescriptorHeap::allocate(size_t size, bool shaderVisible)
{
    std::vector<Page>* targetPages = shaderVisible ? &shaderPages_ : &pages_;
    if (targetPages->empty())
    {
        targetPages->emplace_back(createNewPage(shaderVisible));
    }

    auto& page = targetPages->back();
    auto desc = page.heap->GetDesc();

    if (page.offset + size > desc.NumDescriptors)
    {
        targetPages->emplace_back(createNewPage(shaderVisible));
        page = targetPages->back();
        desc = page.heap->GetDesc();
    }

    check(page.offset + size <= desc.NumDescriptors);

    Descriptor d;
    d.cpuAt = page.heap->GetCPUDescriptorHandleForHeapStart();
    d.gpuAt = page.heap->GetGPUDescriptorHandleForHeapStart();
    d.stride = stride_;
    d.heap = page.heap.get();

    d = d.next(page.offset);
    page.offset += size;

    return d;
}

DescriptorHeap::Page DescriptorHeap::createNewPage(bool shaderVisible)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc = {};
    desc.NumDescriptors = 256;
    desc.Type = type_;
    desc.Flags = shaderVisible ?
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ID3D12DescriptorHeap* heap;
    if (FAILED(device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))))
    {
        GF_LOG_WARN("Descriptor heap allocation error.");
        throw Direct3DException("Failed to create ID3D12DescriptorHeap.");
    }
    heap->SetName(L"DescriptorHeap");

    Page page;
    page.heap = makeComPtr(heap);
    page.offset = 0;

    return page;
}

DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type_)
    : type(type_)
{
}

Descriptor DescriptorAllocator::operator ()(size_t size, bool shaderVisible)
{
    return renderSystem.descriptorHeap(type).allocate(size, shaderVisible);
}

GF_NAMESPACE_END