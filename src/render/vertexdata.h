#ifndef GAMEFRIENDS_VERTEXDATA_H
#define GAMEFRIENDS_VERTEXDATA_H

#include "../windowing/windowsinc.h"
#include "../engine/pixelformat.h"
#include "foundation/sortedvector.h"
#include "foundation/prerequest.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <utility>

GF_NAMESPACE_BEGIN

struct Semantics
{
    static const std::string POSITION;
    static const std::string COLOR;
    static const std::string NORMAL;
    static const std::string TEXCOORD;
};

enum class PrimitiveTopology
{
    lines,
    lineStrip,
    triangles,
    triangleStrip
};

class VertexData
{
private:
    struct VertexBuffer
    {
        std::string semantics;
        size_t index;
        PixelFormat format;
        ComPtr<ID3D12Resource> buffer;
        std::shared_ptr<void> uploadData;
        size_t dataSize;
    };
    
    struct VertComp
    {
        bool operator ()(const VertexBuffer& a, const VertexBuffer& b) const
        {
            if (a.semantics == b.semantics)
            {
                return a.index < b.index;
            }
            return a.semantics < b.semantics;
        }
    };

    struct IndexBuffer
    {
        D3D12_INDEX_BUFFER_VIEW view = {};
        ComPtr<ID3D12Resource> buffer;
        std::shared_ptr<void> uploadData;
        size_t dataSize;
    };

    SortedVector<VertexBuffer, VertComp> vertices_;
    IndexBuffer indices_;
    D3D12_PRIMITIVE_TOPOLOGY topology_;

    std::vector<D3D12_RESOURCE_BARRIER> barriers_;
    std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElems_;

public:
    void setVertices(const std::string& semantics, size_t index, const void* data, size_t size, PixelFormat format);
    void setIndices(const unsigned short* data, size_t size);
    void setTopology(PrimitiveTopology pt);
    void upload(ID3D12GraphicsCommandList& list);
    bool drawableState(ID3D12GraphicsCommandList& list);

    auto vertexBuffers() const ->
        decltype(std::make_pair(std::cbegin(vertexBufferViews_), std::cend(vertexBufferViews_)))
    {
        return std::make_pair(std::cbegin(vertexBufferViews_), std::cend(vertexBufferViews_));
    }

    D3D12_INDEX_BUFFER_VIEW indexBuffer() const;
    D3D12_PRIMITIVE_TOPOLOGY primitiveTopology() const;
    D3D12_INPUT_LAYOUT_DESC inputLayout() const;
};

GF_NAMESPACE_END

#endif