#pragma once
#include <AL/al.h>
#include <AL/alc.h>
#include <dr_libs/dr_wav.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class AudioData {
    public:
    ALuint ID;
    const char *AudioPath;

    std::vector<float> rmsFrames;          // Loudness per frame
    std::vector<float> peakFrames;         // Peak amplitude per frame
    float maxAmplitude = 0.0f;
    float sampleRate = 0.0f;
    float totalDuration = 0.0f;
    int channels = 0;
    int totalSamples = 0;
    AudioData(const char *audioPath) : AudioPath(audioPath) {
        //Source is where the sound comes from, and a buffer is the data of the audio
        alGenBuffers(1, &ID);

        //Get audio data

        unsigned int Channels, SampleRate;
        drwav_uint64 TotalPCMFrameCount;
        drwav_int16 *SampleData = drwav_open_file_and_read_pcm_frames_s16(AudioPath, &Channels, &SampleRate, &TotalPCMFrameCount, NULL);
        if (SampleData == NULL) {
            std::cerr << "PCM frame read failure\n";

            alcMakeContextCurrent(0);
            
            return;
        }

        this->sampleRate = (float)SampleRate;
        this->channels = Channels;
        this->totalSamples = TotalPCMFrameCount * Channels;
        this->totalDuration = (float)TotalPCMFrameCount / (float)SampleRate;

        int frameSize = SampleRate / 20; // 20 frames per second (adjust as needed)
        int numFrames = (TotalPCMFrameCount + frameSize - 1) / frameSize;
        rmsFrames.reserve(numFrames);
        peakFrames.reserve(numFrames);

        for (int frame = 0; frame < numFrames; frame++) {
            int startIdx = frame * frameSize * Channels;
            int endIdx = std::min(startIdx + frameSize * Channels, (int)TotalPCMFrameCount * Channels);

            float sumSquares = 0.0f;
            float peak = 0.0f;

            for (int i = startIdx; i < endIdx; i++) {
                float sample = (float)SampleData[i] / 32768.0f; // maximum number for float_16
                sumSquares += sample * sample;
                if (std::abs(sample) > peak) peak = std::abs(sample);
            }

            int sampleCount = endIdx - startIdx;
            if (sampleCount > 0) {
                float rms = (sampleCount > 0) ? sqrtf(sumSquares / sampleCount) : 0.0f;
                rmsFrames.push_back(rms);
                peakFrames.push_back(peak);
                if (peak > maxAmplitude) maxAmplitude = peak;
            }
        }

        //Get format
        ALenum format;
        if (Channels == 1) {
            format = AL_FORMAT_MONO16;
        } else if (Channels == 2) {
            format = AL_FORMAT_STEREO16;
        } else {
            std::cerr << "OpenAL does not support more than 2 channels (Mono or Stereo) for playback\n";

            alcMakeContextCurrent(0);
            drwav_free(SampleData, NULL);

            return;
        }
        //Get size allocation
        size_t TotalPCMByteCount = TotalPCMFrameCount * Channels * sizeof(drwav_int16);
        //Add the data
        alBufferData(ID, format, SampleData, TotalPCMByteCount, SampleRate);
        //Freeing memory
        drwav_free(SampleData, NULL);
    }

    glm::vec3 Position = glm::vec3(0.0);
    bool Looping = false;
    float Pitch = 1.0;
    float Gain = 1.0;
    glm::vec3 Velocity = glm::vec3(0.0);
    glm::vec3 NormalizedVec = glm::vec3(0.0);

    void SetSourcePosition(glm::vec3 position) {
        Position = position;
    }

    void IsSourceLooping(bool looping) {
        Looping = looping;
    }

    void SetSourcePitch(float pitch) {
        Pitch = pitch;
    }

    void SetSourceGain(float gain) {
        Gain = gain;
    }

    void SetSourceVelocity(glm::vec3 velocity) {
        Velocity = velocity;
    }

    void SetSourceDirection(glm::vec3 normalizedVec) {
        NormalizedVec = normalizedVec;
    }

    float GetLoudnessAtTime(float time) {
        if (rmsFrames.empty()) return 0.0f;
        float frameIndex = time * (rmsFrames.size() / totalDuration);
        int idx = (int)frameIndex;
        if (idx >= rmsFrames.size()) idx = rmsFrames.size() - 1;
        return rmsFrames[idx];
    }

    float GetPeakAtTime(float time) {
        if (peakFrames.empty()) return 0.0f;
        float frameIndex = time * (peakFrames.size() / totalDuration);
        int idx = (int)frameIndex;
        if (idx >= peakFrames.size()) idx = peakFrames.size() - 1;
        return peakFrames[idx];
    }
};