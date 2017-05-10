#include "texture.h"
#include "scene.h"
#include "../render/pixelbuffer.h"
#include "../render/gpucommand.h"
#include "../engine/codec.h"
#include "../engine/logging.h"

GF_NAMESPACE_BEGIN

MediaTexture::MediaTexture(const EnginePath& path)
    : Resource(path)
    , image_()
    , resource_()
{
}

PixelBuffer& MediaTexture::resource()
{
    return *resource_;
}

bool MediaTexture::loadImpl()
{
    try
    {
        image_ = decodeBmp(path());
    }
    catch (const Exception& e)
    {
        GF_LOG_WARN("Failed to load texture {}. {}", osPath(), e.msg());
        return false;
    }

    PixelBufferSetup setup = {};
    setup.width = image_->width();
    setup.height = image_->height();
    setup.arrayLength = 1;
    setup.mipLevels = 0;
    setup.baseFormat = PixelFormat::RGBA8_unorm;
    setup.srvFormat = PixelFormat::RGBA8_unorm;
    setup.state = PixelBufferState::copyDest;
    resource_ = std::make_unique<PixelBuffer>(setup);

    PixelUpload upload;
    upload.data = image_->pixelArray();
    upload.width = setup.width;
    upload.height = setup.height;
    upload.pixelSize = sizeofPixelFormat(setup.baseFormat);

    auto& copy = sceneAppContext.copyCommandBuilder();
    auto& graphics = sceneAppContext.graphicsCommandBuilder();
    copy.uploadPixels(upload, *resource_);
    graphics.transition(*resource_, PixelBufferState::copyDest, PixelBufferState::genericRead);

    return true;
}

void MediaTexture::unloadImpl()
{
    image_.reset();
    resource_.reset();
}

GF_NAMESPACE_END