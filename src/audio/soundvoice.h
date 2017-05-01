#ifndef GAMEFRIENDS_SOUNDVOICE_H
#define GAMEFRIENDS_SOUNDVOICE_H

#include "audioinc.h"
#include "../foundation/prerequest.h"

GF_NAMESPACE_BEGIN

extern const int SOUND_LOOP_INFINITE;

class SoundVoice
{
private:
    ComWeakPtr<IXAudio2> owner_;
    VoicePtr<IXAudio2SourceVoice> voice_;

public:
    SoundVoice(const ComWeakPtr<IXAudio2>& owner, const SoundFormat& fmt);

    void submitBuffer(const unsigned char* data, size_t size, size_t loops);
    void start();
    void stop();
    void flush();
    size_t numQueuedBuffers();
    float frequencyRatio();
    void setFrequencyRatio(float ratio);
    void setOutputMatrix(size_t numSrcChannels, size_t numDestChannels, const float* levelMatrix);
};

GF_NAMESPACE_END

#endif