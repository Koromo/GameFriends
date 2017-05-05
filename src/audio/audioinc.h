#ifndef GAMEFRIENDS_XAUDIOSUPPORT_H
#define GAMEFRIENDS_XAUDIOSUPPORT_H

#define _XM_NO_INTRINSICS_ 1; // For x3daudio.h

#include "../windowing/windowsinc.h" // inc

#include "foundation/exception.h"
#include "foundation/prerequest.h"

#include <xaudio2.h> // inc
#include <x3daudio.h> //inc

#include <memory>
#include <string>

GF_NAMESPACE_BEGIN

using SoundFormat = WAVEFORMATEX;

constexpr size_t MAX_CHANNELS_PER_VOICE = 8;

class XAudioException : public Exception
{
public:
    explicit XAudioException(const std::string& msg)
        : Exception(msg) {}
};

template <class T>
using VoicePtr = std::shared_ptr<T>;

template <class T>
VoicePtr<T> makeVoicePtr(const ComWeakPtr<IXAudio2> owner, T* p)
{
    struct Deleter
    {
        ComWeakPtr<IXAudio2> owner;
        void operator()(T* p)
        {
            if (!owner.expired())
            {
                p->DestroyVoice();
            }
        }
    };

    return VoicePtr<T>(p, Deleter{ owner });
}

GF_NAMESPACE_END

#endif