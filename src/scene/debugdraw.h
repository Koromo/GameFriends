#ifndef GAMEFRIENDS_DEBUGDRAW_H
#define GAMEFRIENDS_DEBUGDRAW_H

#include "../engine/resource.h"
#include "foundation/color.h"
#include "foundation/vector3.h"
#include "foundation/prerequest.h"
#include <vector>
#include <memory>

GF_NAMESPACE_BEGIN

class Material;
struct RenderCamera;
class VertexData;

class DebugDraw
{
private:
    std::vector<Vector3> positions_;
    std::vector<Color> colors_;
    ResourceInterface<Material> material_;
    std::shared_ptr<VertexData> vertex_;

public:
    void startup() noexcept(false);
    void shutdown();

    void line(const Vector3& from, const Vector3& to, const Color& color);

    void drawDebugs(const RenderCamera& camera);
};

extern DebugDraw debugDraw;

GF_NAMESPACE_END

#endif