#include "soundvoice.h"
#include "../engine/logging.h"

GF_NAMESPACE_BEGIN

const int SOUND_LOOP_INFINITE = XAUDIO2_LOOP_INFINITE + 1;

SoundVoice::SoundVoice(const ComWeakPtr<IXAudio2>& owner, const SoundFormat& fmt)
    : owner_(owner)
    , voice_()
{
    IXAudio2SourceVoice* voice;
    if (FAILED(owner_.lock()->CreateSourceVoice(&voice, &fmt)))
    {
        GF_LOG_WARN("Failed to create the source voice.");
    }
    voice_ = makeVoicePtr(owner_, voice);
}

void SoundVoice::submitBuffer(const unsigned char* data, size_t size, size_t loops)
{
    if (owner_.expired() || !voice_)
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

    if (FAILED(voice_->SubmitSourceBuffer(&buffer)))
    {
        GF_LOG_WARN("Failed to submit audio buffers.");
    }
}

void SoundVoice::start()
{
    if (owner_.expired() || !voice_)
    {
        return;
    }

    XAUDIO2_VOICE_STATE state;
    voice_->GetState(&state);
    if (state.BuffersQueued > 0)
    {
        if (FAILED(voice_->Start(0)))
        {
            GF_LOG_WARN("Failed to start audio buffers.");
        }
    }
}

void SoundVoice::stop()
{
    if (owner_.expired() || !voice_)
    {
        return;
    }
    if (FAILED(voice_->Stop()))
    {
        GF_LOG_WARN("Failed to stop audio buffers.");
    }
}

void SoundVoice::flush()
{
    if (owner_.expired() || !voice_)
    {
        return;
    }
    if (FAILED(voice_->FlushSourceBuffers()))
    {
        GF_LOG_WARN("Failed to flush audio buffers.");
    }
}

size_t SoundVoice::numQueuedBuffers()
{
    if (owner_.expired() || !voice_)
    {
        return 0;
    }
    XAUDIO2_VOICE_STATE state = {};
    voice_->GetState(&state);
    return state.BuffersQueued;
}

float SoundVoice::frequencyRatio()
{
    float r = 0;
    if (!owner_.expired() && voice_)
    {
        voice_->GetFrequencyRatio(&r);
    }
    return r;
}

void SoundVoice::setFrequencyRatio(float ratio)
{
    if (owner_.expired() || !voice_)
    {
        return;
    }
    if (FAILED(voice_->SetFrequencyRatio(ratio)))
    {
        GF_LOG_WARN("Failed to change frequency ratio of source voice.");
    }
}

void SoundVoice::setOutputMatrix(size_t numSrcChannels, size_t numDestChannels, const float* levelMatrix)
{
    if (owner_.expired() || !voice_)
    {
        return;
    }
    if (FAILED(voice_->SetOutputMatrix(nullptr, numSrcChannels, numDestChannels, levelMatrix)))
    {
        GF_LOG_WARN("Failed to change output matrix of source voice.");
    }
}

GF_NAMESPACE_END