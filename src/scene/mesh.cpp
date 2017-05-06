#include "mesh.h"
#include "material.h"
#include "scene.h"
#include "../engine/fbximport.h"
#include "foundation/exception.h"
#include <algorithm>

GF_NAMESPACE_BEGIN

Mesh::Mesh(const std::string& path)
    : Resource(path)
    , vertexData_()
    , subMeshes_()
{
}

void Mesh::setVertexData(const std::shared_ptr<VertexData>& vertexData)
{
    vertexData_ = vertexData;
}

void Mesh::addSubMesh(const SubMesh& sm)
{
    const auto r = std::equal_range(std::begin(subMeshes_), std::end(subMeshes_), sm, subMeshes_.comp());
    if (r.first == r.second)
    {
        subMeshes_.insert(sm);
    }
    else
    {
        *r.first = sm;
    }
}

SubMesh& Mesh::subMesh(const std::string& name)
{
    const auto it = std::lower_bound(std::begin(subMeshes_), std::end(subMeshes_), SubMesh{ name }, subMeshes_.comp());
    check(it != std::end(subMeshes_) && it->name == name);
    return *it;
}

std::shared_ptr<VertexData> Mesh::vertexData()
{
    return vertexData_;
}

GF_NAMESPACE_END