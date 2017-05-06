#ifndef GAMEFRIENDS_MESH_H
#define GAMEFRIENDS_MESH_H

#include "../engine/resource.h"
#include "../engine/filesystem.h"
#include "foundation/sortedvector.h"
#include "foundation/prerequest.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

GF_NAMESPACE_BEGIN

class VertexData;
class Material;

struct SubMesh
{
    std::string name;
    bool indexed;
    size_t offset;
    size_t count;
    ResourceInterface<Material> material;
};

class Mesh : public Resource
{
private:
    std::shared_ptr<VertexData> vertexData_;
    
    struct SubMeshComp
    {
        bool operator()(const SubMesh& a, const SubMesh& b)
        {
            return a.name < b.name;
        }
    };
    SortedVector<SubMesh, SubMeshComp> subMeshes_;

public:
    explicit Mesh(const FilePath& path);

    void setVertexData(const std::shared_ptr<VertexData>& vertexData);
    void addSubMesh(const SubMesh& sm);

    SubMesh& subMesh(const std::string& name);

    std::shared_ptr<VertexData> vertexData();

    auto subMeshes()
        -> decltype(std::make_pair(std::cbegin(subMeshes_), std::cend(subMeshes_)))
    {
        return std::make_pair(std::cbegin(subMeshes_), std::cend(subMeshes_));
    }

private:
    void loadImpl();
    void unloadImpl();
};

GF_NAMESPACE_END

#endif