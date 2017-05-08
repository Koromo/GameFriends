#include "../scene/mesh.h"
#include "../scene/material.h"
#include "../scene/scene.h"
#include "../render/vertexdata.h"
#include "pixelformat.h"
#include "resource.h"
#include "logging.h"
#include "foundation/metaprop.h"
#include "foundation/exception.h"

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

bool Mesh::loadImpl()
{
    MetaPropFile file;

    try
    {
        file.read(path().os);
    }
    catch (const Exception& e)
    {
        GF_LOG_WARN("Failed to load mesh {}. {}", path().os, e.msg());
        return false;
    }

    vertexData_ = std::make_shared<VertexData>();

    try
    {
        std::vector<float> floats;
        std::vector<unsigned short> ints;

        // @Vertex
        enforce<MeshLoadException>(file.hasGroup("Vertex"), ".mesh requires @Vertex.");
        const auto& Vertex = file.group("Vertex");

        enforce<MeshLoadException>(Vertex.hasProp("Topology"), ".mesh requires Topology: in @Vertex.");
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

            enforce<MeshLoadException>(SubMeshGroup.hasProp("Name"), ".mesh requires Name: in @SubMesh.");
            const auto name = SubMeshGroup.prop("Name")[0];

            enforce<MeshLoadException>(SubMeshGroup.hasProp("Material"), ".mesh requires Material: in @SubMesh.");
            const auto materialPath = SubMeshGroup.prop("Material")[0];

            enforce<MeshLoadException>(SubMeshGroup.hasProp("Indexed"), ".mesh requires Indexed: in @SubMesh.");
            const auto indexed = !!SubMeshGroup.prop("Indexed").getInt(0);

            enforce<MeshLoadException>(SubMeshGroup.hasProp("Range"), ".mesh requires Range: in @SubMesh.");
            const auto& Range = SubMeshGroup.prop("Range");

            const auto material = resourceManager.template obtain<Material>(materialPath);
            material->load();
            enforce<MaterialLoadException>(material->ready(), "Failed to load material.");

            SubMesh subMesh;
            subMesh.name = name;
            subMesh.material = material;
            subMesh.indexed = indexed;
            subMesh.offset = Range.getInt(0);
            subMesh.count = Range.getInt(1);

            addSubMesh(subMesh);
        }
    }
    catch (const ResourceException& e)
    {
        GF_LOG_WARN("Failed to load mesh {}. {}", path().os, e.msg());
        unloadImpl();
        return false;
    }
    
    auto& copy = sceneAppContext.copyCommandBuilder();
    auto& graphics = sceneAppContext.graphicsCommandBuilder();
    copy.uploadVertices(*vertexData_);
    graphics.drawableState(*vertexData_);

    return true;
}

void Mesh::unloadImpl()
{
    subMeshes_.clear();
    vertexData_.reset();
}

GF_NAMESPACE_END