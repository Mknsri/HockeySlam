#ifndef GAME_MAIN_H
#define GAME_MAIN_H

#include <stdint.h>
#include <cmath>

#include "debug/debug_log.h"
#include "debug/debug_assert.h"

#include "game_math.h"
#include "game_memory.h"
#include "asset/asset_shader.h"
#include "asset/asset_decal.h"
#include "asset/asset_model.h"
#include "asset/asset_text.h"
#include "asset/asset_obj.h"
#include "asset/asset_dds.h"
#include "asset/asset_anim.h"
#include "asset/asset_model.h"
#include "asset/asset_gradient.h"
#include "asset/asset_shader.h"
#include "game_entity.h"
#include "game_camera.h"
#include "game_light.h"
#include "game_input.h"

#include "../ogl/ogl_main.h"

#define SIZE_KB(value) ((value)*1024UL)
#define SIZE_MB(value) (SIZE_KB(value) * 1024UL)
#define SIZE_GB(value) (SIZE_MB(value) * 1024UL)
#define SIZE_TB(value) (SIZE_GB(value) * 1024UL)

using Asset::animation;
using Asset::file;
using Asset::font;
using Asset::model;
using Asset::ogl_shader;
using Asset::ogl_shader_program;
using Asset::texture;

struct game_sound_output_buffer
{
  int16_t* SampleOut;
  int SamplesPerSecond;
  int SampleCount;
};

#define PLATFORM_READ_FILE(name) void name(const char* path, uint8_t* memory)
typedef PLATFORM_READ_FILE(platform_read_file);
PLATFORM_READ_FILE(ReadFileStub) {}

#define PLATFORM_WRITE_FILE(name)                                              \
  void name(const char* path, void* memory, size_t size)
typedef PLATFORM_WRITE_FILE(platform_write_file);
PLATFORM_WRITE_FILE(WriteFileStub) {}

#define PLATFORM_GET_FILE_SIZE(name) size_t name(const char* path)
typedef PLATFORM_GET_FILE_SIZE(platform_get_file_size);
PLATFORM_GET_FILE_SIZE(GetFileSizeStub)
{
  return 0;
}

struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(void* data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);
struct platform_work_queue_job
{
  platform_work_queue_callback* Callback;
  void* Data;
};

#define PLATFORM_ADD_WORK_QUEUE_ENTRY(name)                                    \
  void name(platform_work_queue* queue,                                        \
            platform_work_queue_callback* callback,                            \
            void* data)
typedef PLATFORM_ADD_WORK_QUEUE_ENTRY(platform_add_work_queue_entry);

#define PLATFORM_COMPLETE_ALL_QUEUE_WORK(name)                                 \
  void name(platform_work_queue* queue)
typedef PLATFORM_COMPLETE_ALL_QUEUE_WORK(platform_complete_all_queue_work);

struct game_memory
{
  bool Initialized;

  uint64_t PermanentStorageSize;
  void* PermanentStorage;
  uint64_t TransientStorageSize;
  void* TransientStorage;

  game_memory_allocator PermanentStorageAllocator;
  game_memory_allocator TransientStorageAllocator;

  platform_read_file* ReadFile;
  platform_write_file* WriteFile;
  platform_get_file_size* GetFileSize;

  platform_add_work_queue_entry* AddWorkEntry;
  platform_complete_all_queue_work* CompleteAllQueueWork;
  platform_work_queue* WorkQueue;

  platform_log* Log;
};

#define GAME_MAIN(name)                                                        \
  void name(game_memory& gameMemory,                                           \
            game_window_info windowInfo,                                       \
            render_context& renderContext,                                     \
            game_input_buffer& inputBuffer,                                    \
            float deltaTime)
typedef GAME_MAIN(game_main);
GAME_MAIN(GameMainStub) {}

#define GAME_GET_SOUND_SAMPLES(name)                                           \
  void name(game_memory* gameMemory, game_sound_output_buffer* soundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub) {}

#endif // !GAME_MAIN_H
