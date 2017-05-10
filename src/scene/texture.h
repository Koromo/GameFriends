#ifndef GAMEFRIENDS_TEXTURE_H
#define GAMEFRIENDS_TEXTURE_H

#include "../engine/resource.h"
#include "../engine/filesystem.h"
#include "foundation/prerequest.h"
#include <string>
#include <memory>

GF_NAMESPACE_BEGIN

class Image;
class PixelBuffer;

class MediaTexture : public Resource
{
private:
    std::shared_ptr<Image> image_;
    std::unique_ptr<PixelBuffer> resource_;

public:
    explicit MediaTexture(const EnginePath& path);
    PixelBuffer& resource();

private:
    bool loadImpl();
    void unloadImpl();
};

GF_NAMESPACE_END

#endif
