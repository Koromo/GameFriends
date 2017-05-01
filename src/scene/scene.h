#ifndef GAMEFRIENDS_SCENE_H
#define GAMEFRIENDS_SCENE_H

#include "../render/gpucommand.h"
#include "../render/renderstate.h"
#include "../engine/resource.h"
#include "../foundation/sortedvector.h"
#include "../foundation/matrix44.h"
#include "../foundation/prerequest.h"
#include <memory>
#include <vector>

GF_NAMESPACE_BEGIN

class GpuCommandBuilder;
class PixelBuffer;
class Mesh;

struct RenderEntity
{
    ResourceInterface<Mesh> mesh;
    Matrix44 worldMatrix;
};

struct RenderCamera
{
    Matrix44 view;
    Matrix44 proj;
    Viewport viewport;
};

class RenderWorld
{
private:
    SortedVector<std::shared_ptr<RenderEntity>> entities_;

public:
    void addEntity(const std::shared_ptr<RenderEntity>& entity);
    void removeEntity(const std::shared_ptr<RenderEntity>& entity);
    void clearEntities();

    void draw(const RenderCamera& camera);
};

class SceneAppContext
{
private:
    struct FrameResource
    {
        std::shared_ptr<GpuCommands> graphicsCommands;
        FenceValue fenceValue_;
    };
    std::vector<FrameResource> frameResources_;
    std::shared_ptr<GpuCommandBuilder> graphicsCommandBuilder_;

    std::shared_ptr<GpuCommands> copyCommands_;
    std::shared_ptr<GpuCommandBuilder> copyCommandBuilder_;

    std::unique_ptr<PixelBuffer> depthTarget_;

public:
    void startup();
    void shutdown();

    GpuCommandBuilder& graphicsCommandBuilder();
    GpuCommandBuilder& copyCommandBuilder();

    PixelBuffer& depthTarget();
    PixelBuffer& backBuffer();

    void executeCommandsAndPresent();
};

extern SceneAppContext sceneAppContext;


GF_NAMESPACE_END

#endif