#ifndef WIN_MAIN_H
#define WIN_MAIN_H

static int INPUT_BUFFER_SIZE = 100;

struct win_offscreen_buffer
{
  BITMAPINFO Info;
  void* Memory;
  int Width;
  int Height;
  int BytesPerPixel;
  int Pitch;
};

struct win_window_dimensions
{
  int Width;
  int Height;
};

struct win_sound_output
{
  int SamplesPerSecond;
  int BytesPerSample;
  DWORD SecondaryBufferSize;
  int RunningSampleIndex;
  DWORD SafetyMarginBytes;
};

struct win_game_code
{
  HMODULE GameDLL;
  FILETIME FileTimestamp;
  game_main* GameMain;
  game_get_sound_samples* GetSoundSamples;

  bool Valid;
};

struct win_renderer_code
{
  HMODULE RendererDLL;
  FILETIME FileTimestamp;
  renderer_main* RendererMain;
  renderer_load_extensions* RendererLoadExtensions;

  bool Valid;
};

struct win_playback_input
{
  double TimeOffset;
  game_input Input;
};

struct win_playback_loop
{
  HANDLE RecordFileHandle;
  HANDLE PlaybackFileHandle;

  bool Recording;
  bool Playing;

  uint64_t MemorySize;
  void* Memory;

  double Start;
  double CurrentTime;
  win_playback_input NextInput;
};

struct platform_work_queue
{
  HANDLE SemaphoreHandle;
  uint32_t volatile WorkAdded;
  uint32_t volatile WorkCompleted;
  uint32_t volatile WriteIndex;
  uint32_t volatile ReadIndex;
  platform_work_queue_job Jobs[256];
};

#endif