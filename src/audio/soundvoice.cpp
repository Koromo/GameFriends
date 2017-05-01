#include "soundvoice.h"

GF_NAMESPACE_BEGIN

const int SOUND_LOOP_INFINITE = XAUDIO2_LOOP_INFINITE + 1;

SoundVoice::SoundVoice(const ComWeakPtr<IXAudio2>& owner, const SoundFormat& fmt)
    : owner_(owner)
    , voice_()
{
    IXAudio2SourceVoice* voice;
    verify<XAudioException>(owner_.lock()->CreateSourceVoice(&voice, &fmt),
        "Failed to create the IXAudio2SourceVoice.");
    voice_ = makeVoicePtr(owner_, voice);
}

void SoundVoice::submitBuffer(const unsigned char* data, size_t size, size_t loops)
{
    if (owner_.expired())
    {
        return;
    }

    if (loops > SOUND_LOOP_INFINITE)
    {
        loops = SOUND_LOOP_INFINITE;
    }

    XAUDIO2_BUFFER buffer = {};
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = size;
    buffer.pAudioData = data;
    buffer.PlayBegin = 0;
    buffer.PlayLength = 0;
    buffer.LoopBegin = 0;
    buffer.LoopLength = 0;
    buffer.LoopCount = loops - 1;
    buffer.pContext = this;

    verify<XAudioException>(voice_->SubmitSourceBuffer(&buffer),
        "Failed to submit buffers.");
}

void SoundVoice::start()
{
    if (owner_.expired())
    {
        return;
    }

    XAUDIO2_VOICE_STATE state;
    voice_->GetState(&state);
    if (state.BuffersQueued > 0)
    {
        verify<XAudioException>(voice_->Start(0),
            "Failed to start the voice.");
    }
}

void SoundVoice::stop()
{
    if (owner_.expired())
    {
        return;
    }
    verify<XAudioException>(voice_->Stop(),
        "Failed to stop the voice.");
}

void SoundVoice::flush()
{
    if (owner_.expired())
    {
        return;
    }
    verify<XAudioException>(voice_->FlushSourceBuffers(),
        "Failed to flush buffers.");
}

size_t SoundVoice::numQueuedBuffers()
{
    XAUDIO2_VOICE_STATE state = {};
    voice_->GetState(&state);
    return state.BuffersQueued;
}

float SoundVoice::frequencyRatio()
{
    float r = 1;
    if (!owner_.expired())
    {
        voice_->GetFrequencyRatio(&r);
    }
    return r;
}

void SoundVoice::setFrequencyRatio(float ratio)
{
    if (owner_.expired())
    {
        return;
    }
    verify<XAudioException>(voice_->SetFrequencyRatio(ratio),
        "Failed to apply frequency ratio to the voice.");
}

void SoundVoice::setOutputMatrix(size_t numSrcChannels, size_t numDestChannels, const float* levelMatrix)
{
    verify<XAudioException>(
        voice_->SetOutputMatrix(nullptr, numSrcChannels, numDestChannels, levelMatrix),
        "Failed to set the output matrix.");
}

GF_NAMESPACE_END