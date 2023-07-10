#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <aaudio/AAudio.h>
#include <string>
#include <ctime>
#include <cmath>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <atomic>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ogl/ogl_main.h>
#include <ogl/ogl_main.cpp>
#include <game/game_main.cpp>

#include <android/log.h>
#define LOG_TAG "game-activity.cpp"
#define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static const int INPUT_BUFFER_SIZE = 100;

static ANativeWindow *window = nullptr;
static AAssetManager *manager = nullptr;
EGLDisplay display;
EGLContext context;
EGLSurface surface;
game_input_buffer InputBuffer = {};
render_context RenderContext = {};
bool gameRunning = false;
bool openGlReady = false;

const char* strip_path(const char* path) {
    char* lastSlash = (char*) path;
    char* fullPath = (char*) path;
    while (*fullPath++) {
        if (*fullPath == '/') {
            lastSlash = fullPath;
        }
    }

    return lastSlash + 1;
}

static long ns_to_ms(const long ns) {
    return ns / (long)1E6;

}

static long ms_to_ns(const long ms) {
    return ms * (long)1E6;
}

static float ms_to_s(const long ms) {
    return (float) ms / 1E3;
}

static long s_to_ns(const float s) {
    return (long)(s * 1E9);
}


static long now_ms() {
    timespec res = {};
    clock_gettime(CLOCK_MONOTONIC, &res);
    long secToMs = 1000 * res.tv_sec;
    return secToMs + ns_to_ms(res.tv_nsec);

}

PLATFORM_LOG(AndroidLog) {
    char str[1024];

    va_list argptr;
    va_start(argptr, fmt);
    int ret = vsnprintf(str, sizeof(str), fmt, argptr);
    va_end(argptr);

    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", str);
}

PLATFORM_READ_FILE(ReadFile) {
    AAsset* asset = AAssetManager_open(manager, strip_path(path), AASSET_MODE_STREAMING);
    off_t size = AAsset_getLength(asset);
    off_t start = 0;

    off_t read = AAsset_read(asset, memory, (size_t) size);

    AAsset_close(asset);

    HOKI_ASSERT(read == size);
}

PLATFORM_GET_FILE_SIZE(GetFileSize) {
    AAsset* asset = AAssetManager_open(manager, strip_path(path), AASSET_MODE_STREAMING);
    off_t size = AAsset_getLength(asset);

    AAsset_close(asset);

    return (size_t) size;
}

game_window_info setupGLES() {
    EGLint majorVersion;
    EGLint minorVersion;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, &majorVersion, &minorVersion);
    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, // color components - 8 bit
            EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8,
#if 0
            // Disabled MSAA for now
            EGL_SAMPLE_BUFFERS, 1,
            EGL_SAMPLES, 4,  // 4x MSAA
#endif
            EGL_NONE };

    EGLConfig  config;
    EGLint  numConfigs;
    // The application chooses the configuration it desires
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    surface = eglCreateWindowSurface(display, config, window, nullptr);
    // As soon as we picked a EGLConfig, we can safely reconfigure
    // the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    // Change the format and size of the window buffers.
    // 0,0 -> Buffer dimensions = Screen resolution
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    context = eglCreateContext(display, config, nullptr, contextAttribs);
    eglMakeCurrent(display, surface, surface, context);
    EGLint width;
    EGLint height;
    eglQuerySurface(display,surface,EGL_WIDTH,&width);
    eglQuerySurface(display,surface,EGL_HEIGHT,&height);

    game_window_info info = {};
    info.Width = width;
    info.Height = height;

    return info;
}

void handle_cmd(android_app *state, int32_t cmd) {
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
            window = state->window;
            setupGLES();
            openGlReady = true;
            LOG_ERROR("APP_CMD_INIT_WINDOW");
            break;

        case APP_CMD_RESUME:
            LOG_ERROR("APP_CMD_RESUME");
            gameRunning = true;
            break;

        case APP_CMD_PAUSE:
            gameRunning = false;
            break;

        case APP_CMD_TERM_WINDOW:
            LOG_ERROR("APP_CMD_TERM_WINDOW");
            window = nullptr;
            openGlReady = false;
            RenderContext = {};
            break;

        default: break;
    }
}

float startX = FLT_MAX;
float startY = FLT_MAX;
bool handle_touch_state_change(android_app *state, AInputEvent* event) {
    switch (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK)
    {
        case AMOTION_EVENT_ACTION_DOWN: {
            game_input newInputState = {};
            newInputState.State.Code = INPUT_CODE_CURSOR_CLICK;
            newInputState.State.Flags |= (INPUT_STATE_IS_DOWN | INPUT_STATE_CHANGED);

            float w = (float) ANativeWindow_getWidth(window);
            float h = (float) ANativeWindow_getHeight(window);
            float x = (AMotionEvent_getRawX(event, 0) / w);
            float y = (AMotionEvent_getRawY(event, 0) / h);
            newInputState.Position.X = x;
            newInputState.Position.Y = y;
            InputBuffer.Inputs[InputBuffer.InputCount++] = newInputState;

            startX = x;
            startY = y;
        } break;

        case AMOTION_EVENT_ACTION_UP: {
            game_input newInputState = {};

            newInputState.State.Code = INPUT_CODE_CURSOR_CLICK;
            newInputState.State.Flags |= (INPUT_STATE_IS_UP | INPUT_STATE_CHANGED);
            InputBuffer.Inputs[InputBuffer.InputCount++] = newInputState;

            startX = FLT_MAX;
            startY = FLT_MAX;

        } break;

        case AMOTION_EVENT_ACTION_POINTER_UP:
            if (AMotionEvent_getPointerCount(event) == 2) {
                game_input newInputState = {};
                newInputState.State.Code = INPUT_CODE_NUM_2;
                newInputState.State.Flags |= INPUT_STATE_IS_DOWN;
                InputBuffer.Inputs[InputBuffer.InputCount++] = newInputState;

            }
            break;

        case AMOTION_EVENT_ACTION_MOVE: // Handled in handle_input
        default: return false;
    }

    return true;
}

int32_t handle_input(android_app *state, AInputEvent* event) {

    switch (AInputEvent_getType(event))
    {
        case AINPUT_EVENT_TYPE_KEY:

            break;

        case AINPUT_EVENT_TYPE_MOTION:
        {
            if (handle_touch_state_change(state, event)) {
                break;
            }

            float w = (float) ANativeWindow_getWidth(window);
            float h = (float) ANativeWindow_getHeight(window);
            float x = (AMotionEvent_getRawX(event, 0) / w);
            float y = (AMotionEvent_getRawY(event, 0) / h);

            if (fabs(x) < 0.0001f) {
                x = 0.0f;
            }
            if (fabs(y) < 0.0001f) {
                y = 0.0f;
            }

            game_input newInputState = {};
            newInputState.State.Code = INPUT_CODE_CURSOR_MOVE;
            newInputState.Position.X = x - startX;
            newInputState.Position.Y = y - startY;
            InputBuffer.Inputs[InputBuffer.InputCount++] = newInputState;

            startX = x;
            startY = y;

        } break;

        default: break;
    }

    return 0;
}

struct android_sound_output {
    int SamplesPerSecond;
    int BytesPerSample;
    size_t BufferSize;
    uint64_t RunningSampleIndex;
    void* Samples;
    int BufferSizeSamples;
    int ExpectedSamplesPerFrame;
};

struct android_frame_sound_data {
    game_memory* Memory;
    AAudioStream* AAStream;
    android_sound_output* AndroidSoundOutput;
};

void AndroidFillAAudioStream(AAudioStream* stream, android_sound_output* androidSoundOutput, game_sound_output_buffer* gameSoundOutputBuffer) {
    int64_t wroteSamples = AAudioStream_write(stream, gameSoundOutputBuffer->SampleOut, gameSoundOutputBuffer->SampleCount, 1000000L);
    HOKI_ASSERT(wroteSamples == gameSoundOutputBuffer->SampleCount);
    androidSoundOutput->RunningSampleIndex += wroteSamples;
}

void DEBUG_AndroidWriteSineWave(android_frame_sound_data* data) {
    game_sound_output_buffer gameSoundBuffer = {};
    gameSoundBuffer.SamplesPerSecond = data->AndroidSoundOutput->SamplesPerSecond;
    gameSoundBuffer.SampleOut = (int16_t *)(data->AndroidSoundOutput->Samples);
    gameSoundBuffer.SampleCount = data->AndroidSoundOutput->ExpectedSamplesPerFrame;

    GameGetSoundSamples(data->Memory, &gameSoundBuffer);
    AndroidFillAAudioStream(data->AAStream, data->AndroidSoundOutput, &gameSoundBuffer);
}

PLATFORM_WORK_QUEUE_CALLBACK(AndroidProcessSound) {
    android_frame_sound_data* soundData = (android_frame_sound_data*) data;
    android_sound_output* androidOutput = soundData->AndroidSoundOutput;

    game_sound_output_buffer gameSoundBuffer = {};
    gameSoundBuffer.SamplesPerSecond = androidOutput->SamplesPerSecond;
    gameSoundBuffer.SampleCount = androidOutput->ExpectedSamplesPerFrame * 2;
    int64_t samplesWritten =
            androidOutput->RunningSampleIndex % androidOutput->BufferSizeSamples;
    gameSoundBuffer.SampleOut = (int16_t*)androidOutput->Samples + samplesWritten;

    int64_t writtenCursor = androidOutput->RunningSampleIndex;
    int64_t framesPlayed = AAudioStream_getFramesWritten(soundData->AAStream);

    if (writtenCursor < (framesPlayed + gameSoundBuffer.SampleCount)) {
        if (samplesWritten + gameSoundBuffer.SampleCount > androidOutput->BufferSizeSamples) {
            uint64_t samplesOver =
                    (samplesWritten + gameSoundBuffer.SampleCount) -
                    androidOutput->BufferSizeSamples;

            game_sound_output_buffer splitGameSoundBuffer = {};
            splitGameSoundBuffer.SamplesPerSecond =
                    androidOutput->SamplesPerSecond;
            splitGameSoundBuffer.SampleCount = samplesOver;
            splitGameSoundBuffer.SampleOut = (int16_t *) (androidOutput->Samples);

            gameSoundBuffer.SampleCount -= samplesOver;

            GameGetSoundSamples(soundData->Memory, &gameSoundBuffer);
            AndroidFillAAudioStream(soundData->AAStream, androidOutput, &gameSoundBuffer);
            if (samplesOver > 0) {
                GameGetSoundSamples(soundData->Memory, &splitGameSoundBuffer);
                AndroidFillAAudioStream(soundData->AAStream, androidOutput, &splitGameSoundBuffer);
            }
        } else {
            GameGetSoundSamples(soundData->Memory, &gameSoundBuffer);
            AndroidFillAAudioStream(soundData->AAStream, androidOutput, &gameSoundBuffer);
        }
    }
}

aaudio_data_callback_result_t AndroidAudioCallback(
        AAudioStream *stream,
        void *userData,
        void *audioData,
        int32_t numFrames) {
    game_memory* memory = (game_memory*) userData;

    if (!memory->Initialized) {
        for (int32_t i = 0; i < numFrames * 2; i++) {
            ((int16_t*) audioData)[i] = 0;
        }
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    game_sound_output_buffer gameSoundBuffer = {};
    gameSoundBuffer.SamplesPerSecond = AAudioStream_getSampleRate(stream);
    gameSoundBuffer.SampleOut = (int16_t*) audioData;
    gameSoundBuffer.SampleCount = numFrames;

    // Write samples directly into the audioData array.
    GameGetSoundSamples((game_memory*)userData, &gameSoundBuffer);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}


struct platform_work_queue
{
    sem_t Semaphore;
    uint32_t volatile WorkAdded;
    std::atomic_uint32_t volatile WorkCompleted;
    uint32_t volatile WriteIndex;
    std::atomic_uint32_t volatile ReadIndex;
    platform_work_queue_job Jobs[256];
};

PLATFORM_ADD_WORK_QUEUE_ENTRY(AndroidPushJob)
{
    uint32_t nextIndex = (queue->WriteIndex + 1) % ARRAY_SIZE(queue->Jobs);
    HOKI_ASSERT(nextIndex != queue->ReadIndex);
    platform_work_queue_job* newJob = queue->Jobs + queue->WriteIndex;
    *newJob = {};
    newJob->Callback = callback;
    newJob->Data = data;
    queue->WorkAdded++;
    asm volatile("": : :"memory");
    queue->WriteIndex = nextIndex;
    sem_post(&queue->Semaphore);
}

bool AndroidDoNextQueueWorkEntry(platform_work_queue* queue) {
    bool queueHasWork = false;

    uint32_t currentIndex = queue->ReadIndex.load();
    uint32_t nextIndex = (currentIndex + 1) % ARRAY_SIZE(queue->Jobs);
    if (currentIndex != queue->WriteIndex) {
        if (queue->ReadIndex.compare_exchange_weak(currentIndex, nextIndex)) {
            platform_work_queue_job job = queue->Jobs[currentIndex];
            job.Callback(job.Data);
            queue->WorkCompleted++;
        }

        queueHasWork = true;
    }

    return queueHasWork;
}

[[noreturn]] void* AndroidDoWork(void *data) {
    platform_work_queue* queue = (platform_work_queue*) data;

    for (;;) {
        if (!AndroidDoNextQueueWorkEntry(queue)) {
            sem_wait(&queue->Semaphore);
        }
    }
}

PLATFORM_COMPLETE_ALL_QUEUE_WORK(AndroidCompleteAllWork)
{
    while (queue->WorkAdded != queue->WorkCompleted) {
        AndroidDoNextQueueWorkEntry(queue);
    }

    int workAdded = queue->WorkAdded;
    int workCompleted = queue->WorkCompleted;
    HOKI_ASSERT(workAdded == workCompleted);

    queue->WorkAdded = 0;
    queue->WorkCompleted.store(0);
}


void android_main(struct android_app* state) {
    state->onAppCmd = handle_cmd;
    state->onInputEvent = handle_input;

    manager = state->activity->assetManager;

    size_t permanentSize = SIZE_MB(64);
    size_t transientSize = SIZE_MB(128);

    platform_work_queue workQueue = {};
    workQueue.WorkCompleted.store(0);

    game_memory memory = {};
    memory.Initialized = false;
    memory.PermanentStorageSize = permanentSize;
    memory.PermanentStorage = malloc(permanentSize);
    memory.TransientStorageSize = transientSize;
    memory.TransientStorage = malloc(transientSize);
    memory.GetFileSize = &GetFileSize;
    memory.ReadFile = &ReadFile;
    memory.Log = &AndroidLog;
    memory.AddWorkEntry = AndroidPushJob;
    memory.CompleteAllQueueWork = AndroidCompleteAllWork;
    memory.WorkQueue = &workQueue;

    InputBuffer = {};
    InputBuffer.Inputs = (game_input*) malloc(sizeof(game_input) * INPUT_BUFFER_SIZE);

    long availableCores = sysconf(_SC_NPROCESSORS_CONF);
    for (long i = 0; i < availableCores; i++) {
        pthread_t thread;
        pthread_create(&thread, nullptr, AndroidDoWork, &workQueue);
    }

    int gameTargetUpdateHz = 60;
    float targetSecondsPerFrame = 1.0f / (float)gameTargetUpdateHz;
    long lastFrameTime = now_ms();
    float frameTime = targetSecondsPerFrame;

    android_sound_output androidSoundOutput = {};
    androidSoundOutput.SamplesPerSecond = 48000;
    androidSoundOutput.BytesPerSample = sizeof(int16_t) * 2;
    androidSoundOutput.BufferSize =
            (androidSoundOutput.SamplesPerSecond * androidSoundOutput.BytesPerSample);
    androidSoundOutput.Samples = malloc(androidSoundOutput.BufferSize);
    androidSoundOutput.BufferSizeSamples =
            androidSoundOutput.BufferSize / androidSoundOutput.BytesPerSample;
    androidSoundOutput.ExpectedSamplesPerFrame =
            (androidSoundOutput.SamplesPerSecond / gameTargetUpdateHz);

    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);
#if __ANDROID_API__ > 28
    AAudioStreamBuilder_setContentType(builder, AAUDIO_CONTENT_TYPE_SONIFICATION);
    AAudioStreamBuilder_setUsage(builder, AAUDIO_USAGE_GAME);
#endif
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setDataCallback(builder, AndroidAudioCallback, &memory);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
    AAudioStreamBuilder_setChannelCount(builder, 2);
    AAudioStreamBuilder_setSampleRate(builder, androidSoundOutput.SamplesPerSecond);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setBufferCapacityInFrames(builder, androidSoundOutput.BufferSizeSamples);

    AAudioStream *stream;
    AAudioStreamBuilder_openStream(builder, &stream);
    AAudioStream_setBufferSizeInFrames(stream, androidSoundOutput.ExpectedSamplesPerFrame * 6);
    AAudioStreamBuilder_delete(builder);
    androidSoundOutput.BufferSizeSamples = AAudioStream_getBufferCapacityInFrames(stream);
    HOKI_ASSERT(AAudioStream_getBufferCapacityInFrames(stream) == androidSoundOutput.BufferSizeSamples);
    uint64_t bfSize = AAudioStream_getBufferSizeInFrames(stream);
    //androidSoundOutput.ExpectedSamplesPerFrame = bfSize;
    //HOKI_ASSERT(bfSize == androidSoundOutput.ExpectedSamplesPerFrame * 3);
    HOKI_ASSERT(AAudioStream_getFormat(stream) == AAUDIO_FORMAT_PCM_I16);

    AAudioStream_requestStart(stream);

    android_frame_sound_data soundData = {};
    soundData.AAStream = stream;
    soundData.Memory = &memory;
    soundData.AndroidSoundOutput = &androidSoundOutput;
    
    int events;
    android_poll_source *pSource;
    do {
        if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
            if (pSource) {
                pSource->process(state, pSource);
            }
        }

        if (gameRunning && openGlReady) {
            int32_t w = ANativeWindow_getWidth(window);
            int32_t h = ANativeWindow_getHeight(window);
            game_window_info localWindowInfo = {};
            localWindowInfo.Width = w;
            localWindowInfo.Height = h;
            RenderContext.Commands.Count = 0;
            GameMain(memory,
                     localWindowInfo,
                     RenderContext,
                     InputBuffer,
                     frameTime);

            RendererMain(localWindowInfo, RenderContext);
            eglSwapBuffers(display, surface);


            long frameEndTime = now_ms();
            frameTime = ms_to_s(frameEndTime - lastFrameTime);

            if (frameTime < targetSecondsPerFrame) {
                timespec ts = {};
                timespec tsRem = {};
                ts.tv_sec = 0;
                ts.tv_nsec =  s_to_ns(targetSecondsPerFrame - frameTime);
                nanosleep(&ts, &tsRem);
                while (frameTime < targetSecondsPerFrame) {
                    frameEndTime = now_ms();
                    frameTime = ms_to_s(frameEndTime - lastFrameTime);
                }
            }

            if (frameTime > 1.0f) {
                // Cap the elapsed seconds in case we are in a breakpoint
                frameTime = targetSecondsPerFrame;
            }

            lastFrameTime = now_ms();
        }

    } while (!state->destroyRequested);

    if (display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(display, surface, surface, context);
        if (context != EGL_NO_CONTEXT){
            eglDestroyContext(display, context);
        }
        if (surface != EGL_NO_SURFACE){
            eglDestroySurface(display, surface);
        }
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
}