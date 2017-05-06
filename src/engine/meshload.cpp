#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../scene/scene.h"
#include "../render/vertexdata.h"
#include "pixelformat.h"
#include "resource.h"
#include "foundation/metaprop.h"

GF_NAMESPACE_BEGIN

namespace
{
    template <class T>
    bool getFloatBuffer(const MetaPropGroup& Vertex, const std::string& propName, std::vector<T>& buffer)
    {
        if (Vertex.hasProp(propName))
        {
            buffer.clear();
            const auto& V = Vertex.prop(propName);
            const auto size = V.size();
            buffer.resize(size);
            for (size_t i = 0; i < size; ++i)
            {
                buffer[i] = V.getFloat(i);
            }
            return true;
        }
        return false;
    }

    template <class T>
    bool getIntBuffer(const MetaPropGroup& Vertex, const std::string& propName, std::vector<T>& buffer)
    {
        if (Vertex.hasProp(propName))
        {
            buffer.clear();
            const auto& V = Vertex.prop(propName);
            const auto size = V.size();
            buffer.resize(size);
            for (size_t i = 0; i < size; ++i)
            {
                buffer[i] = V.getInt(i);
            }
            return true;
        }
        return false;
    }
}

void Mesh::loadImpl()
{
    MetaPropFile file;
    file.read(path().os);

    vertexData_ = std::make_shared<VertexData>();
    std::vector<float> floats;
    std::vector<unsigned short> ints;

    const auto& Vertex = file.group("Vertex");
    vertexData_->setTopology(static_cast<PrimitiveTopology>(Vertex.prop("Topology").getInt(0)));

    if (getFloatBuffer(Vertex, "V", floats))
    {
        vertexData_->setVertices(Semantics::POSITION, 0, floats.data(), floats.size() * sizeof(float), PixelFormat::RGB32_float);
    }
    if (getFloatBuffer(Vertex, "N", floats))
    {
        vertexData_->setVertices(Semantics::NORMAL, 0, floats.data(), floats.size() * sizeof(float), PixelFormat::RGB32_float);
    }
    if (getFloatBuffer(Vertex, "C", floats))
    {
        vertexData_->setVertices(Semantics::COLOR, 0, floats.data(), floats.size() * sizeof(float), PixelFormat::RGBA32_float);
    }
    if (getFloatBuffer(Vertex, "U", floats))
    {
        vertexData_->setVertices(Semantics::TEXCOORD, 0, floats.data(), floats.size() * sizeof(float), PixelFormat::RG32_float);
    }

    if (getIntBuffer(Vertex, "I", ints))
    {
        vertexData_->setIndices(ints.data(), ints.size() * sizeof(unsigned short));
    }

    for (int i = 0; file.hasGroup("SubMesh" + std::to_string(i)); ++i)
    {
        const auto& SubMeshGroup = file.group("SubMesh" + std::to_string(i));

        const auto materialPath = SubMeshGroup.prop("Material")[0];
        const auto material = resourceTable.template obtain<Material>(materialPath);
        material->load();

        const auto& Range = SubMeshGroup.prop("Range");

        SubMesh subMesh;
        subMesh.name = SubMeshGroup.prop("Name")[0];
        subMesh.material = material;
        subMesh.indexed = !!SubMeshGroup.prop("Indexed").getInt(0);
        subMesh.offset = Range.getInt(0);
        subMesh.count = Range.getInt(1);

        addSubMesh(subMesh);
    }
    
    auto& copy = sceneAppContext.copyCommandBuilder();
    auto& graphics = sceneAppContext.graphicsCommandBuilder();
    copy.uploadVertices(*vertexData_);
    graphics.drawableState(*vertexData_);
}

void Mesh::unloadImpl()
{
    subMeshes_.clear();
    vertexData_.reset();
}

GF_NAMESPACE_END