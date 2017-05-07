#include "vertexdata.h"
#include "linearallocator.h"
#include "rendersystem.h"
#include "d3dsupport.h"
#include "foundation/exception.h"
#include <algorithm>
#include <cstring>

GF_NAMESPACE_BEGIN

const std::string Semantics::POSITION = "POSITION";
const std::string Semantics::COLOR = "COLOR";
const std::string Semantics::NORMAL = "NORMAL";
const std::string Semantics::TEXCOORD = "TEXCOORD";

void VertexData::setVertices(const std::string& semantics, size_t index, const void* data, size_t size, PixelFormat format)
{
    VertexBuffer key;
    key.semantics = semantics;
    key.index = index;

    auto r = std::equal_range(std::begin(vertices_), std::end(vertices_), key, vertices_.comp());
    if (r.first == r.second)
    {
         r.first = vertices_.insert(key);
    }
    const auto found = r.first;

    const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);

    ID3D12Resource* buffer;
    if (FAILED(renderSystem.nativeDevice().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer))))
    {
        /// LOG
        vertices_.erase(found);
        return;
    }
    buffer->SetName(L"VertexBuffer");

    found->buffer = makeComPtr(buffer);
    found->format = format;
    found->dataSize = size;
    found->uploadData.reset(new char[size]);
    std::memcpy(found->uploadData.get(), data, size);

    vertexBufferViews_.clear();
    inputElems_.clear();
    vertexBufferViews_.reserve(vertices_.size());
    inputElems_.reserve(vertices_.size());

    int i = 0;
    for (const auto& v : vertices_)
    {
        D3D12_VERTEX_BUFFER_VIEW view = {};
        view.BufferLocation = v.buffer->GetGPUVirtualAddress();
        view.SizeInBytes = v.dataSize;
        view.StrideInBytes = sizeofPixelFormat(v.format);
        vertexBufferViews_.emplace_back(view);

        D3D12_INPUT_ELEMENT_DESC elem = {};
        elem.SemanticName = v.semantics.c_str();
        elem.SemanticIndex = v.index;
        elem.Format = D3DMappings::DXGI_FORMAT_(v.format);
        elem.InputSlot = i;
        elem.AlignedByteOffset = 0;
        elem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        elem.InstanceDataStepRate = 0;
        inputElems_.emplace_back(elem);

        ++i;
    }
}

void VertexData::setIndices(const unsigned short* data, size_t size)
{
    const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    const auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);

    ID3D12Resource* buffer;
    if (FAILED(renderSystem.nativeDevice().CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer))))
    {
        /// LOG
        indices_ = {};
        return;
    }
    buffer->SetName(L"IndexBuffer");

    indices_.buffer = makeComPtr(buffer);
    indices_.dataSize = size;
    indices_.uploadData.reset(new char[size]);
    std::memcpy(indices_.uploadData.get(), data, size);

    indices_.view.BufferLocation = buffer->GetGPUVirtualAddress();
    indices_.view.SizeInBytes = size;
    indices_.view.Format = DXGI_FORMAT_R16_UINT;
}

void VertexData::setTopology(PrimitiveTopology pt)
{
    topology_ = D3DMappings::PRIMITIVE_TOPOLOGY(pt);
}

void VertexData::upload(ID3D12GraphicsCommandList& list)
{
    struct Upload
    {
        ID3D12Resource* dest;
        std::shared_ptr<void>& srcData;
        size_t srcSize;
        D3D12_RESOURCE_STATES afterState;
    };

    std::vector<Upload> uploads;

    for (auto& v : vertices_)
    {
        if (v.uploadData)
        {
            const auto dest = v.buffer.get();
            uploads.emplace_back(Upload{ dest, v.uploadData, v.dataSize, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER });
        }
    }
    if (indices_.uploadData)
    {
        const auto dest = indices_.buffer.get();
        uploads.emplace_back(Upload{ dest, indices_.uploadData, indices_.dataSize, D3D12_RESOURCE_STATE_INDEX_BUFFER });
    }

    if (!uploads.empty())
    {
        barriers_.reserve(uploads.size());

        for (auto& u : uploads)
        {
            const auto intermediateSize = GetRequiredIntermediateSize(u.dest, 0, 1);
            const auto intermediate = CpuAllocator()(static_cast<size_t>(intermediateSize), sizeof(float));

            D3D12_SUBRESOURCE_DATA srcData = {};
            srcData.pData = u.srcData.get();
            srcData.RowPitch = static_cast<LONG_PTR>(u.srcSize);
            srcData.SlicePitch = srcData.RowPitch;

            const auto uploadedSize = UpdateSubresources<1>(&list, u.dest, intermediate.resource, intermediate.offset, 0, 1, &srcData);
            if (uploadedSize != u.srcSize)
            {
                /// LOG
            }

            barriers_.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(u.dest, D3D12_RESOURCE_STATE_COPY_DEST, u.afterState));

            u.srcData.reset();
        }
    }
}

void VertexData::drawableState(ID3D12GraphicsCommandList& list)
{
    if (!barriers_.empty())
    {
        list.ResourceBarrier(barriers_.size(), barriers_.data());
        barriers_.clear();
    }
}

D3D12_INDEX_BUFFER_VIEW VertexData::indexBuffer() const
{
    return indices_.view;
}

D3D12_PRIMITIVE_TOPOLOGY VertexData::primitiveTopology() const
{
    return topology_;
}

D3D12_INPUT_LAYOUT_DESC VertexData::inputLayout() const
{
    D3D12_INPUT_LAYOUT_DESC il = {};
    il.NumElements = inputElems_.size();
    il.pInputElementDescs = inputElems_.data();
    return il;
}

GF_NAMESPACE_END