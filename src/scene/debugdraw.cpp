#include "debugdraw.h"
#include "../scene/scene.h"
#include "../scene/material.h"
#include "../render/pixelbuffer.h"
#include "../render/vertexdata.h"
#include "../render/gpucommand.h"
#include "../render/drawcall.h"
#include "../engine/pixelformat.h"
#include "foundation/matrix44.h"

GF_NAMESPACE_BEGIN

namespace
{
    const auto DEBUG_MATERIAL = "media/debugdraw.material";
}

void DebugDraw::startup()
{
    material_ = resourceTable.template obtain<Material>(DEBUG_MATERIAL);
    material_->load();
}

void DebugDraw::shutdown()
{
    material_->unload();
    positions_.clear();
    colors_.clear();
    vertex_.reset();
}

void DebugDraw::drawDebugs(const RenderCamera& camera)
{
    if (positions_.empty())
    {
        return;
    }

    vertex_ = std::make_shared<VertexData>();
    vertex_->setTopology(PrimitiveTopology::lines);
    vertex_->setVertices(Semantics::POSITION, 0, positions_.data(),  positions_.size() * sizeof(Vector3), PixelFormat::RGB32_float);
    vertex_->setVertices(Semantics::COLOR, 0, colors_.data(), colors_.size() * sizeof(Color), PixelFormat::RGBA32_float);

    auto& copy = sceneAppContext.copyCommandBuilder();
    auto& graphics = sceneAppContext.graphicsCommandBuilder();
    auto& backBuffer = sceneAppContext.backBuffer();

    copy.uploadVertices(*vertex_);
    graphics.drawableState(*vertex_);

    material_->setFloat4x4("ViewProj", (camera.view * camera.proj).transpose());

    auto& pass = material_->passNth(0);
    auto drawCall = pass.drawCallBase();
    drawCall.setRenderTarget(backBuffer);
    drawCall.setVertex(*vertex_, 0, positions_.size(), 0, 1);
    drawCall.setViewport(camera.viewport);
    drawCall.setViewport(camera.viewport);
    graphics.triggerDrawCall(drawCall);

    positions_.clear();
    colors_.clear();
}

void DebugDraw::line(const Vector3& from, const Vector3& to, const Color& color)
{
    positions_.emplace_back(from);
    positions_.emplace_back(to);
    colors_.emplace_back(color);
    colors_.emplace_back(color);
}

DebugDraw debugDraw;

GF_NAMESPACE_END