#include "audiomanager.h"
#include "soundvoice.h"
#include <vector>

GF_NAMESPACE_BEGIN

namespace
{
    std::vector<float> local_sound_volume_matrix(MAX_CHANNELS_PER_VOICE * 2, 0.8f);
}

const float* const LOCAL_SOUND_VOLUME_MATRIX = local_sound_volume_matrix.data();

AudioManager audioManager;

void AudioManager::startup()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IXAudio2* xAudio;
    verify<XAudioException>(XAudio2Create(&xAudio),
        "Failed to create the IXAudio2.");
    xAudio_ = makeComPtr(xAudio);

    IXAudio2MasteringVoice* masteringVoice;
    verify<XAudioException>(xAudio_->CreateMasteringVoice(&masteringVoice),
        "Failed to create the IXAudio2MasteringVoice.");
    masteringVoice_ = makeVoicePtr(xAudio_, masteringVoice);

    DWORD channelMask;
    masteringVoice_->GetChannelMask(&channelMask);

    XAUDIO2_VOICE_DETAILS details;
    masteringVoice_->GetVoiceDetails(&details);

    deviceDetails_.channelMask = channelMask;
    deviceDetails_.numSrcCannels = details.InputChannels;
    deviceDetails_.numDestCannels = 2; /// TODO:

    verify<XAudioException>(
        X3DAudioInitialize(deviceDetails_.channelMask, X3DAUDIO_SPEED_OF_SOUND, x3DAudio_),
        "Failed to initialize the X3DAudio.");

    listener_ = {};
    listenerActive_ = true;
    levelMatrixMemory_.fill(0.8f);
}

void AudioManager::shutdown()
{
    xAudio_->StopEngine();
    masteringVoice_.reset();
    xAudio_.reset();
    CoUninitialize();
}

AudioDeviceDetails AudioManager::deviceDetails() const
{
    return deviceDetails_;
}

std::shared_ptr<SoundVoice> AudioManager::createSoundVoice(const SoundFormat& fmt)
{
    return std::make_shared<SoundVoice>(xAudio_, fmt);
}

void AudioManager::setListener(const SoundListener& listener)
{
    listener_ = listener;
}

void AudioManager::listenerExists(bool b)
{
    listenerActive_ = b;
}

namespace
{
    X3DAUDIO_VECTOR toX3DAudioVector(const Vector3& v)
    {
        return{ v.x, v.y, v.z };
    }
}

void AudioManager::calculateDSPSettings(const SoundEmitter& emitter, DSPSettings& out)
{
    out.numSrcChannels = deviceDetails_.numSrcCannels; 
    out.numDstChannels = deviceDetails_.numDestCannels; 

    if (!listenerActive_)
    {
        levelMatrixMemory_.fill(0);
        out.volumeMatrix = levelMatrixMemory_.data();
        return;
    }

    X3DAUDIO_LISTENER lis = {};
    lis.OrientFront = toX3DAudioVector(listener_.orientation * Vector3::UNIT_Z);
    lis.OrientTop = toX3DAudioVector(listener_.orientation * Vector3::UNIT_Y);
    lis.Position = toX3DAudioVector(listener_.position);
    lis.Velocity = toX3DAudioVector(listener_.velocity);

    X3DAUDIO_EMITTER emi = {};

    /// TODO:
    emi.InnerRadius = 30;
    emi.ChannelCount = 1;
    emi.pVolumeCurve = const_cast<X3DAUDIO_DISTANCE_CURVE*>(&X3DAudioDefault_LinearCurve);
    emi.CurveDistanceScaler = 500;
    emi.DopplerScaler = 1;

    emi.OrientFront = toX3DAudioVector(emitter.orientation * Vector3::UNIT_Z);
    emi.OrientTop = toX3DAudioVector(emitter.orientation * Vector3::UNIT_Y);
    emi.Position = toX3DAudioVector(emitter.position);
    emi.Velocity = toX3DAudioVector(emitter.velocity);

    X3DAUDIO_DSP_SETTINGS dsp;
    dsp.pMatrixCoefficients = levelMatrixMemory_.data();
    dsp.pDelayTimes = NULL;
    dsp.SrcChannelCount = deviceDetails_.numSrcCannels;
    dsp.DstChannelCount = deviceDetails_.numDestCannels;

    X3DAudioCalculate(x3DAudio_, &lis, &emi, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dsp);
    out.volumeMatrix = dsp.pMatrixCoefficients;
    out.dopplerFactor = dsp.DopplerFactor;
}

GF_NAMESPACE_END