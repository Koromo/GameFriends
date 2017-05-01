#ifndef GAMEFRIENDS_MESH_H
#define GAMEFRIENDS_MESH_H

#include "../engine/resource.h"
#include "../foundation/prerequest.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

GF_NAMESPACE_BEGIN

class VertexData;
class Material;

struct Surface
{
    std::shared_ptr<VertexData> vertices;
    size_t indexOffset;
    size_t indexCount;

    ResourceInterface<Material> material;
};

class Mesh : public Resource
{
private:
    std::vector<Surface> surfaces_;

public:
    explicit Mesh(const std::string& path);

    size_t addSurface(const Surface& s);
    Surface& surface(size_t id);

    auto surfaces() const
        -> decltype(std::make_pair(std::cbegin(surfaces_), std::cend(surfaces_)))
    {
        return std::make_pair(std::cbegin(surfaces_), std::cend(surfaces_));
    }

private:
    void loadImpl();
    void unloadImpl();
};

GF_NAMESPACE_END

#endif