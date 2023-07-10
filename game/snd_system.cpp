#include "snd_system.h"

namespace SoundSystem {

wav LoadSound(std::string path, game_memory* memory)
{
  wav result = {};
  file loadedFile = Asset::load_file(path, *memory);

  uint8_t* wavPtr = (uint8_t*)loadedFile.Memory;

  const unsigned char riff[] = {
    *wavPtr++, *wavPtr++, *wavPtr++, *wavPtr++, '\0'
  };
  HOKI_ASSERT(std::strcmp((const char*)riff, "RIFF") == 0);

  uint32_t chunkSize = *(uint32_t*)wavPtr;
  HOKI_ASSERT(chunkSize == (loadedFile.Size - 8));
  wavPtr += sizeof(chunkSize);
  const unsigned char wave[] = {
    *wavPtr++, *wavPtr++, *wavPtr++, *wavPtr++, '\0'
  };
  HOKI_ASSERT(std::strcmp((const char*)wave, "WAVE") == 0);

  const unsigned char fmt[] = {
    *wavPtr++, *wavPtr++, *wavPtr++, *wavPtr++, '\0'
  };
  HOKI_ASSERT(std::strcmp((const char*)fmt, "fmt ") == 0);

  uint32_t subchunk1Size = *(uint32_t*)wavPtr;
  HOKI_ASSERT(subchunk1Size == 16 || subchunk1Size == 18);
  wavPtr += sizeof(subchunk1Size);

  uint16_t audioFormat = *(uint16_t*)wavPtr;
  HOKI_ASSERT(audioFormat == 1);
  wavPtr += sizeof(audioFormat);

  uint16_t numChannels = *(uint16_t*)wavPtr;
  wavPtr += sizeof(numChannels);

  uint32_t sampleRate = *(uint32_t*)wavPtr;
  wavPtr += sizeof(sampleRate);

  uint32_t byteRate = *(uint32_t*)wavPtr;
  wavPtr += sizeof(byteRate);

  uint16_t blockAlign = *(uint16_t*)wavPtr;
  wavPtr += sizeof(blockAlign);

  uint16_t bitsPerSample = *(uint16_t*)wavPtr;
  wavPtr += sizeof(bitsPerSample);

  if (subchunk1Size == 18) {
    HOKI_ASSERT(*(uint16_t*)wavPtr == 0);
    wavPtr += 2;
  }

  unsigned char nextChunk[] = {
    *wavPtr++, *wavPtr++, *wavPtr++, *wavPtr++, '\0'
  };
  while (std::strcmp((const char*)nextChunk, "data") != 0) {
    uint32_t nextChunkSize = *(uint32_t*)wavPtr;
    wavPtr += sizeof(nextChunkSize) + nextChunkSize;

    nextChunk[0] = *wavPtr++;
    nextChunk[1] = *wavPtr++;
    nextChunk[2] = *wavPtr++;
    nextChunk[3] = *wavPtr++;
  }

  uint32_t subchunk2Size = *(uint32_t*)wavPtr;
  wavPtr += sizeof(subchunk2Size);

  result.SamplesStart = (int16_t*)wavPtr;
  result.SampleCursor = result.SamplesStart;
  result.SampleSize = bitsPerSample / blockAlign / numChannels;
  result.SamplesSize = subchunk2Size / sizeof(int16_t);
  result.Volume = 1;
  result.Channels = numChannels;

  return result;
}

static void DEBUG_OutputSineWave(game_state* state,
                                 game_sound_output_buffer* soundOutput)
{
  int wavePeriod = soundOutput->SamplesPerSecond / state->Hz;
  int16_t* sampleOut = (int16_t*)soundOutput->SampleOut;
  for (int sampleIndex = 0; sampleIndex < soundOutput->SampleCount;
       sampleIndex++) {
    float sineValue = sinf(state->TSine);
    int16_t sampleValue = (int16_t)(sineValue * state->ToneVolume);
    *sampleOut++ = sampleValue; // L
    *sampleOut++ = sampleValue; // R

    state->TSine += 2.0f * (float)M_PI / (float)wavePeriod;
    if (state->TSine > (2.0f * M_PI)) {
      state->TSine -= 2.0f * (float)M_PI;
    }
  }
}

static void FillSoundBuffer(game_state* state,
                            game_sound_output_buffer* soundOutput,
                            game_memory* memory)
{
  if (!state->SoundEnabled) {
    int16_t* sampleOut = (int16_t*)soundOutput->SampleOut;
    for (int i = 0; i < soundOutput->SampleCount; i++) {
      *sampleOut++ = 0; // L
      *sampleOut++ = 0; // R
    }
    return;
  }

#if HOKI_DEV && 0
  DEBUG_OutputSineWave(state, soundOutput);
  return;
#endif

  static wav wavFile = LoadSound("/res/sounds/song.wav", memory);

  int16_t* sampleOut = (int16_t*)soundOutput->SampleOut;
  for (int sampleIndex = 0; sampleIndex < soundOutput->SampleCount;
       sampleIndex++) {
    for (size_t i = 0; i < wavFile.SampleSize; i++) {
      int16_t sample = *wavFile.SampleCursor++;
      *sampleOut++ = sample / 2;
    }
    if (wavFile.SampleCursor >= wavFile.SamplesStart + wavFile.SamplesSize) {
      wavFile.SampleCursor = wavFile.SamplesStart;
    }
  }
}
}