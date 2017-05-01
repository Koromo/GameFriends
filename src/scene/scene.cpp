#include "scene.h"
#include "mesh.h"
#include "material.h"
#include "../engine/pixelformat.h"
#include "../render/rendersystem.h"
#include "../render/pixelbuffer.h"
#include "../foundation/math.h"
#include "../foundation/color.h"
#include <queue>

GF_NAMESPACE_BEGIN

SceneAppContext sceneAppContext;

void RenderWorld::addEntity(const std::shared_ptr<RenderEntity>& entity)
{
    const auto notFound = std::cend(entities_);
    const auto r = std::equal_range(std::cbegin(entities_), std::cend(entities_), entity, entities_.comp());
    if (r.first == r.second)
    {
        entities_.insert(entity);
    }
}

void RenderWorld::removeEntity(const std::shared_ptr<RenderEntity>& entity)
{
    const auto r = std::equal_range(std::cbegin(entities_), std::cend(entities_), entity, entities_.comp());
    entities_.erase(r.first, r.second);
}

void RenderWorld::clearEntities()
{
    entities_.clear();
}

void RenderWorld::draw(const RenderCamera& camera)
{
    auto& graphics = sceneAppContext.graphicsCommandBuilder();
    auto& backBuffer = sceneAppContext.backBuffer();
    auto& depthTarget = sceneAppContext.depthTarget();

    graphics.clearDepthTarget(depthTarget, 1);

    const auto view_T = transpose(camera.view);
    const auto proj_T = transpose(camera.proj);

    for (const auto& entity : entities_)
    {
        const auto mesh = entity->mesh;
        if (!mesh.useable())
        {
            continue;
        }

        const auto world_T = transpose(entity->worldMatrix);
        for (auto surfaces = mesh->surfaces(); surfaces.first != surfaces.second; ++surfaces.first)
        {
            auto& surface = *surfaces.first;
            if (!surface.vertices || !surface.material.useable())
            {
                continue;
            }

            const auto numPasses = surface.material->numPasses();
            for (size_t n = 0; n < numPasses; ++n)
            {
                auto& pass = surface.material->passNth(n);
                pass.updateNumeric(AutoParameter::WORLD, &world_T, sizeof(Matrix44));
                pass.updateNumeric(AutoParameter::VIEW, &view_T, sizeof(Matrix44));
                pass.updateNumeric(AutoParameter::PROJ, &proj_T, sizeof(Matrix44));

                auto& bindings = pass.shaderBindings();
                auto drawCall = pass.drawCallBase();

                drawCall.setRenderTarget(backBuffer);
                drawCall.setDepthTarget(depthTarget);
                drawCall.setVertexIndexed(*surface.vertices, 0, surface.indexOffset, surface.indexCount, 0, 1);

                graphics.prepareDrawCall(drawCall);
                graphics.setShaderBindings(bindings);
                graphics.setViewport(camera.viewport);
                graphics.triggerDrawCall(drawCall);
            }
        }
    }
}

void SceneAppContext::startup()
{
    auto& graphicsExe = renderSystem.commandExecuter(GpuCommandType::graphics);
    auto& copyExe = renderSystem.commandExecuter(GpuCommandType::copy);
    const auto numFrameBuffers = renderSystem.numBackBuffers();
    const auto frameIndex = renderSystem.currentFrameIndex();

    frameResources_.resize(numFrameBuffers);
    for (size_t i = 0; i < numFrameBuffers; ++i)
    {
        frameResources_[i].graphicsCommands = graphicsExe.createCommands();
        frameResources_[i].fenceValue_ = 0;
    }
    copyCommands_ = copyExe.createCommands();

    graphicsCommandBuilder_ = graphicsExe.createBuilder();
    copyCommandBuilder_ = copyExe.createBuilder();

    graphicsCommandBuilder_->record(*frameResources_[frameIndex].graphicsCommands);
    copyCommandBuilder_->record(*copyCommands_);

    const auto fullySize = renderSystem.fullyViewport();
    PixelBufferSetup depthSetup = {};
    depthSetup.width = static_cast<size_t>(fullySize.width + EPSILON);
    depthSetup.height = static_cast<size_t>(fullySize.height + EPSILON);
    depthSetup.baseFormat = PixelFormat::R32;
    depthSetup.dtvFormat = PixelFormat::D32_float;
    depthSetup.arrayLength = 1;
    depthSetup.mipLevels = 1;
    depthSetup.flags = PBF_AllowDepthTarget | PBF_DenyShaderResource;
    depthSetup.state = PixelBufferState::depthWrite;
    depthTarget_ = std::make_unique<PixelBuffer>(depthSetup, 1.f);

    graphicsCommandBuilder_->transition(backBuffer(), PixelBufferState::present, PixelBufferState::renderTarget);
    graphicsCommandBuilder_->clearRenderTarget(backBuffer(), { 0, 0, 0, 1 });
}

void SceneAppContext::shutdown()
{
    depthTarget_.reset();
    copyCommandBuilder_.reset();
    copyCommands_.reset();
    graphicsCommandBuilder_.reset();
    frameResources_.clear();
}

GpuCommandBuilder& SceneAppContext::graphicsCommandBuilder()
{
    return *graphicsCommandBuilder_;
}

GpuCommandBuilder& SceneAppContext::copyCommandBuilder()
{
    return *copyCommandBuilder_;
}

PixelBuffer& SceneAppContext::depthTarget()
{
    return *depthTarget_;
}

PixelBuffer& SceneAppContext::backBuffer()
{
    return renderSystem.backBuffer(renderSystem.currentFrameIndex());
}

void SceneAppContext::executeCommandsAndPresent()
{
    graphicsCommandBuilder_->transition(backBuffer(), PixelBufferState::renderTarget, PixelBufferState::present);

    const auto frameIndex = renderSystem.currentFrameIndex();

    auto& graphicsExe = renderSystem.commandExecuter(GpuCommandType::graphics);
    auto& copyExe = renderSystem.commandExecuter(GpuCommandType::copy);

    graphicsCommandBuilder_->close();
    copyCommandBuilder_->close();

    const auto copyCompleted = copyExe.execute(*copyCommandBuilder_);
    copyExe.waitForFenceCompletion(copyCompleted);

    frameResources_[frameIndex].fenceValue_ = graphicsExe.execute(*graphicsCommandBuilder_);
    //graphicsExe.waitForFenceCompletion(frameResources_[frameIndex].fenceValue_);

    renderSystem.present();

    const auto newFrameIndex = renderSystem.currentFrameIndex();
    graphicsExe.waitForFenceCompletion(frameResources_[newFrameIndex].fenceValue_);

    frameResources_[newFrameIndex].graphicsCommands->reset();
    graphicsCommandBuilder_->record(*frameResources_[newFrameIndex].graphicsCommands);

    copyCommands_->reset();
    copyCommandBuilder_->record(*copyCommands_);

    graphicsCommandBuilder_->transition(backBuffer(), PixelBufferState::present, PixelBufferState::renderTarget);
    graphicsCommandBuilder_->clearRenderTarget(backBuffer(), { 0, 0, 0, 1 });
}

GF_NAMESPACE_END