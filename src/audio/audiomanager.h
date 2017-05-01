#ifndef GAMEFRIENDS_AUDIOMANAGER_H
#define GAMEFRIENDS_AUDIOMANAGER_H

#include "audioinc.h"
#include "../foundation/vector3.h"
#include "../foundation/quaternion.h"
#include "../foundation/prerequest.h"
#include <memory>
#include <array>

GF_NAMESPACE_BEGIN 

class SoundVoice;

extern const float* const LOCAL_SOUND_VOLUME_MATRIX;

struct AudioDeviceDetails
{
    size_t numSrcCannels;
    size_t numDestCannels;
    unsigned long channelMask;
};

struct SoundListener
{
    Vector3 position;
    Quaternion orientation;
    Vector3 velocity;
};

struct SoundEmitter
{
    Vector3 position;
    Quaternion orientation;
    Vector3 velocity;
};

struct DSPSettings
{
    size_t numSrcChannels;
    size_t numDstChannels;
    const float* volumeMatrix;
    float dopplerFactor;
};

class AudioManager
{
private:
    ComPtr<IXAudio2> xAudio_;
    VoicePtr<IXAudio2MasteringVoice> masteringVoice_;
    AudioDeviceDetails deviceDetails_;
    X3DAUDIO_HANDLE x3DAudio_;

    SoundListener listener_;
    bool listenerActive_;
    std::array<float, MAX_CHANNELS_PER_VOICE * 2> levelMatrixMemory_;

public:
    void startup();
    void shutdown();

    AudioDeviceDetails deviceDetails() const;
        
    std::shared_ptr<SoundVoice> createSoundVoice(const SoundFormat& fmt);

    void setListener(const SoundListener& listener);
    void listenerExists(bool b);
    void calculateDSPSettings(const SoundEmitter& emitter, DSPSettings& out);
};

extern AudioManager audioManager;

GF_NAMESPACE_END

#endif