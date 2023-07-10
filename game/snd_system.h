#pragma once

struct wav
{
  int Volume;
  int Channels;
  int16_t* SampleCursor;
  int16_t* SamplesStart;
  size_t SamplesSize;
  size_t SampleSize;
};