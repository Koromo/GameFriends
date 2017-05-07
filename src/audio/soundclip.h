#ifndef GAMEFRIENDS_SOUNDCLIP_H
#define GAMEFRIENDS_SOUNDCLIP_H

#include "audioinc.h"
#include "../engine/resource.h"
#include "../engine/filesystem.h"
#include "foundation/prerequest.h"
#include <memory>
#include <string>

GF_NAMESPACE_BEGIN

class SoundClip : public Resource
{
private:
    std::unique_ptr<unsigned char[]> data_;
    size_t size_;
    SoundFormat format_;

public:
    SoundClip(const FilePath& path);
    const unsigned char* data() const;
    size_t size() const;
    const SoundFormat& format() const;

private:
    bool loadImpl();
    void unloadImpl();
};

GF_NAMESPACE_END

#endif