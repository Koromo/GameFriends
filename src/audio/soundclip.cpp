#include "soundclip.h"

GF_NAMESPACE_BEGIN

SoundClip::SoundClip(const std::string& path)
    : Resource(path)
    , data_()
    , size_(0)
    , format_()
{
}

const unsigned char* SoundClip::data() const
{
    return data_.get();
}

size_t SoundClip::size() const
{
    return size_;
}

const SoundFormat& SoundClip::format() const
{
    return format_;
}

GF_NAMESPACE_END