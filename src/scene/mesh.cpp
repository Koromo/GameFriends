#include "mesh.h"
#include "material.h"
#include "scene.h"
#include "../engine/fbximport.h"
#include "foundation/exception.h"
#include <algorithm>

GF_NAMESPACE_BEGIN

Mesh::Mesh(const std::string& path)
    : Resource(path)
    , surfaces_()
{
}

size_t Mesh::addSurface(const Surface& s)
{
    surfaces_.emplace_back(s);
    return surfaces_.size();
}

Surface& Mesh::surface(size_t id)
{
    check(id < surfaces_.size());
    return surfaces_[id];
}

void Mesh::loadImpl()
{
    fbxImport.importMesh(path(), *this);
    auto& copy = sceneAppContext.copyCommandBuilder();
    auto& graphics = sceneAppContext.graphicsCommandBuilder();
    for (auto& s : surfaces_)
    {
        copy.uploadVertices(*s.vertices);
        graphics.drawableState(*s.vertices);
    }
}

void Mesh::unloadImpl()
{
    surfaces_.clear();
}

GF_NAMESPACE_END