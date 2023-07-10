#include <windows.h>
#include <xaudio2.h>
#include <tchar.h>
#include <Xinput.h>
#include <dsound.h>
#include <signal.h>
#include <intrin.h>

#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wglext.h>

#include "../game/game_main.h"
#include "../game/debug/debug_assert.h"

#include "win_main.h"

#define UNICODE
#define _UNICODE

static const char WINDOW_TITLE[] = { "HokiCPP" };
static const char WINDOW_CLASS[] = { "HokiCPP" };

static const int WINDOW_WIDTH = 400;
static const int WINDOW_HEIGHT = 600;

static bool GameRunning = true;
static bool mouseLocked = false;
// static win_offscreen_buffer GlobalBackbuffer;
static LPDIRECTSOUNDBUFFER GlobalSoundBuffer;

static win_playback_loop GlobalPlaybackLoop;
static IXAudio2SourceVoice* GlobalSourceVoice;
static game_input_buffer InputBuffer = {};
static render_context RenderContext = {};
static bool DEBUG_StickyWindow = false;
static win_renderer_code GlobalRenderer;

void APIENTRY MessageCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar* message,
                              const void* userParam)
{
  if (type == 0x8251) {
    return;
  }
  char kek[1024];
  sprintf_s(kek,
            sizeof(kek),
            "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type,
            severity,
            message);
  OutputDebugStringA(kek);
}

static void WinLog(const char* format, ...)
{
  char str[1024];

  va_list argptr;
  va_start(argptr, format);
  int ret = vsnprintf(str, sizeof(str), format, argptr);
  va_end(argptr);

  OutputDebugStringA(str);
}

inline DWORD WinGetTime()
{
  DWORD result;
  result = timeGetTime();
  return result;
}

static float WinGetElapsedSeconds(DWORD from, DWORD to)
{
  DWORD difference = to - from;
  float seconds = (float)difference / 1000.0f;
  return seconds;
}

static void WinSetupOpenGL(HWND window,
                           HWND tempWindow,
                           win_renderer_code& rendererCode)
{
  HDC tempHdc = GetDC(tempWindow);

  PIXELFORMATDESCRIPTOR desiredFormat = {};
  desiredFormat.nSize = sizeof(desiredFormat);
  desiredFormat.nVersion = 1;
  desiredFormat.iPixelType = PFD_TYPE_RGBA;
  desiredFormat.dwFlags =
    PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
  desiredFormat.cColorBits = 32;
  desiredFormat.cAlphaBits = 8;
  desiredFormat.cDepthBits = 24;
  desiredFormat.cStencilBits = 8;
  desiredFormat.iLayerType = PFD_MAIN_PLANE;

  int suggestedFormatIndex = ChoosePixelFormat(tempHdc, &desiredFormat);
  PIXELFORMATDESCRIPTOR suggestedFormat = {};
  DescribePixelFormat(
    tempHdc, suggestedFormatIndex, sizeof(suggestedFormat), &suggestedFormat);
  SetPixelFormat(tempHdc, suggestedFormatIndex, &suggestedFormat);

  HGLRC tempContext = wglCreateContext(tempHdc);
  if (wglMakeCurrent(tempHdc, tempContext)) {
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
      (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress(
        "wglCreateContextAttribsARB");
    HOKI_ASSERT(wglCreateContextAttribsARB != nullptr);
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
      (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress(
        "wglChoosePixelFormatARB");
    HOKI_ASSERT(wglChoosePixelFormatARB != nullptr);

    HDC windowDC = GetDC(window);

    const int pixelFormatAttribList[] = {
      WGL_DRAW_TO_WINDOW_ARB,
      GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB,
      GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB,
      GL_TRUE,
      WGL_PIXEL_TYPE_ARB,
      WGL_TYPE_RGBA_ARB,
      WGL_COLOR_BITS_ARB,
      32,
      WGL_DEPTH_BITS_ARB,
      24,
      WGL_STENCIL_BITS_ARB,
      8,
      WGL_SAMPLE_BUFFERS_ARB,
      1, // Number of buffers (must be 1 at time of writing)
      WGL_SAMPLES_ARB,
      4, // Number of samples
      0, // End
    };

    int pixelFormat = 0;
    UINT numFormats = 0;
    wglChoosePixelFormatARB(
      windowDC, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats);
    if (pixelFormat && numFormats) {
      PIXELFORMATDESCRIPTOR PFD;
      DescribePixelFormat(windowDC, pixelFormat, sizeof(PFD), &PFD);
      SetPixelFormat(windowDC, pixelFormat, &PFD);
    } else {
      // Fallback to suggested
      SetPixelFormat(windowDC, suggestedFormatIndex, &suggestedFormat);
    }

    const int contextAttribList[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB,
      3,
      WGL_CONTEXT_MINOR_VERSION_ARB,
      3,
      WGL_CONTEXT_FLAGS_ARB,
      WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      WGL_CONTEXT_PROFILE_MASK_ARB,
      WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0 // List terminator
    };

    HGLRC attribGLRC =
      wglCreateContextAttribsARB(windowDC, 0, contextAttribList);
    wglMakeCurrent(windowDC, attribGLRC);
    wglDeleteContext(tempContext);

    ReleaseDC(tempWindow, tempHdc);
    DestroyWindow(tempWindow);

    if (rendererCode.RendererLoadExtensions() == HOKI_OGL_EXTENSIONS_OK) {
    } else {
      // todo: Problem diagnostics
      HOKI_ASSERT(false);
    }
    ReleaseDC(window, windowDC);
  } else {
    // todo: Problem diagnostics
  }

  // During init, enable debug output
#if HOKI_DEV
  glEnable(GL_DEBUG_OUTPUT);
  PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback =
    (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
  glDebugMessageCallback(MessageCallback, 0);
#endif
}

static void WinPlaybackStartRecord(win_playback_loop* playbackLoop)
{
  char* fileName = "game.pbl";
  playbackLoop->RecordFileHandle =
    CreateFileA(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

  DWORD empty;
  DeviceIoControl(
    playbackLoop->RecordFileHandle, FSCTL_SET_SPARSE, NULL, 0, 0, 0, &empty, 0);

  DWORD bytesToWrite = (DWORD)playbackLoop->MemorySize;
  HOKI_ASSERT(bytesToWrite == playbackLoop->MemorySize);
  DWORD bytesWritten;
  WriteFile(playbackLoop->RecordFileHandle,
            playbackLoop->Memory,
            bytesToWrite,
            &bytesWritten,
            0);
  HOKI_ASSERT(bytesToWrite == bytesWritten);

  playbackLoop->Recording = true;
  playbackLoop->Start = 0;
  playbackLoop->CurrentTime = 0.0f;
}

static void WinPlaybackStopRecord(win_playback_loop* playbackLoop)
{
  CloseHandle(playbackLoop->RecordFileHandle);
  playbackLoop->Recording = false;
}

static void WinPlaybackLoopRecordInput(win_playback_loop* playbackLoop,
                                       game_input_buffer* inputBuffer,
                                       const double elapsedSeconds)
{
  DWORD bytesWritten;

  for (uint32_t i = 0; i < inputBuffer->InputCount; i++) {
    win_playback_input newInput = {};
    newInput.TimeOffset = playbackLoop->CurrentTime;
    newInput.Input = inputBuffer->Inputs[i];
    WriteFile(playbackLoop->RecordFileHandle,
              &newInput,
              sizeof(win_playback_input),
              &bytesWritten,
              0);
  }
  playbackLoop->CurrentTime += elapsedSeconds;
}

static void WinPlaybackStartPlayback(win_playback_loop* playbackLoop)
{
  char* fileName = "game.pbl";
  playbackLoop->PlaybackFileHandle = CreateFileA(
    fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

  DWORD bytesRead;
  DWORD bytesToRead = (DWORD)playbackLoop->MemorySize;
  ReadFile(playbackLoop->PlaybackFileHandle,
           playbackLoop->Memory,
           bytesToRead,
           &bytesRead,
           0);

  HOKI_ASSERT(bytesToRead == bytesRead);

  playbackLoop->CurrentTime = 0.0f;
  playbackLoop->Playing = true;
}

static void WinPlaybackStopPlayback(win_playback_loop* playbackLoop)
{
  CloseHandle(playbackLoop->PlaybackFileHandle);
  playbackLoop->Playing = false;
  playbackLoop->NextInput.TimeOffset = 0;
}

static void WinPlaybackLoopPlayInput(win_playback_loop* playbackLoop,
                                     game_input_buffer* inputBuffer,
                                     const double elapsedSeconds)
{
  DWORD bytesRead = 0;

  while (playbackLoop->NextInput.TimeOffset <= playbackLoop->CurrentTime) {
    inputBuffer->Inputs[inputBuffer->InputCount++] =
      playbackLoop->NextInput.Input;
    ReadFile(playbackLoop->PlaybackFileHandle,
             &playbackLoop->NextInput,
             sizeof(win_playback_input),
             &bytesRead,
             0);
    if (bytesRead != sizeof(win_playback_input)) {
      // Done reading
      WinPlaybackStopPlayback(playbackLoop);
      WinPlaybackStartPlayback(playbackLoop);
      return;
    }
  }
  playbackLoop->CurrentTime += elapsedSeconds;
}

static FILETIME GetFileWriteTime(const char* fileName)
{
  FILETIME fileWriteTime = {};

  WIN32_FILE_ATTRIBUTE_DATA fileData;
  if (GetFileAttributesEx(fileName, GetFileExInfoStandard, &fileData)) {
    fileWriteTime = fileData.ftLastWriteTime;
  }

  return fileWriteTime;
}

static void GetFullPath(const char* relativePath, char* result)
{
  GetModuleFileName(NULL, result, MAX_PATH);

  char* lastSlash = NULL;
  while (*result++) {
    if (*result == '\\' || *result == '/') {
      lastSlash = result + 1;
    }
  }
  while (*relativePath) {
    *lastSlash++ = *relativePath++;
  }
  *lastSlash = '\0';
}

static PLATFORM_GET_FILE_SIZE(WinGetFileSize)
{
  char fullPath[MAX_PATH];
  GetFullPath(path, fullPath);

  wchar_t wString[4096];
  MultiByteToWideChar(CP_UTF8, 0, fullPath, -1, wString, 4096);
  DWORD shortPathLength = GetShortPathNameW(wString, NULL, 0);
  wchar_t shortPath[MAX_PATH];
  GetShortPathNameW(wString, shortPath, shortPathLength);
  DWORD kek = GetLastError();

  HANDLE file = CreateFileW(
    shortPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  HOKI_ASSERT(file != INVALID_HANDLE_VALUE);

  LARGE_INTEGER size;
  GetFileSizeEx(file, &size);
  HOKI_ASSERT(CloseHandle(file) != 0);

  return size.LowPart;
}

static PLATFORM_READ_FILE(WinReadFile)
{
  char fullPath[MAX_PATH];
  GetFullPath(path, fullPath);

  wchar_t wString[4096];
  MultiByteToWideChar(CP_UTF8, 0, fullPath, -1, wString, 4096);
  DWORD shortPathLength = GetShortPathNameW(wString, NULL, 0);
  wchar_t shortPath[MAX_PATH];
  DWORD pathNameResult = GetShortPathNameW(wString, shortPath, shortPathLength);
  HOKI_ASSERT(pathNameResult);
  DWORD kek = GetLastError();

  HANDLE file = CreateFileW(
    shortPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  HOKI_ASSERT(file != INVALID_HANDLE_VALUE);

  LARGE_INTEGER size;
  GetFileSizeEx(file, &size);
  HOKI_ASSERT(size.LowPart != 0);

  DWORD bytesRead = 0;
  ReadFile(file, memory, size.LowPart, &bytesRead, 0);
  HOKI_ASSERT((DWORD)size.LowPart == bytesRead);
  HOKI_ASSERT(CloseHandle(file) != 0);
}

PLATFORM_WRITE_FILE(WinWriteFile)
{

  char fullPath[MAX_PATH];
  GetFullPath(path, fullPath);

  wchar_t wString[4096];
  MultiByteToWideChar(CP_UTF8, 0, fullPath, -1, wString, 4096);

  HANDLE file = CreateFileW(
    wString, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
  HOKI_ASSERT(file != INVALID_HANDLE_VALUE);

  DWORD bytesRead = 0;
  WriteFile(file, memory, (DWORD)size, &bytesRead, 0);
  HOKI_ASSERT((DWORD)size == bytesRead);
  HOKI_ASSERT(CloseHandle(file) != 0);
}

static win_game_code LoadGame(const char* fileName)
{
  win_game_code loadGameResult = {};

  char* tempDLLName = "game_main_loaded.dll";

  CopyFile(fileName, tempDLLName, FALSE);
  HMODULE gameCodeLibrary = LoadLibrary(tempDLLName);
  if (gameCodeLibrary) {
    loadGameResult.GameDLL = gameCodeLibrary;
    loadGameResult.FileTimestamp = GetFileWriteTime(tempDLLName);
    loadGameResult.GameMain =
      (game_main*)GetProcAddress(gameCodeLibrary, "GameMain");
    loadGameResult.GetSoundSamples = (game_get_sound_samples*)GetProcAddress(
      gameCodeLibrary, "GameGetSoundSamples");

    loadGameResult.Valid =
      (loadGameResult.GameMain && loadGameResult.GetSoundSamples);
  }

  if (!loadGameResult.Valid) {
    loadGameResult.GameMain = GameMainStub;
    loadGameResult.GetSoundSamples = GameGetSoundSamplesStub;
  }

  return loadGameResult;
}

static void UnloadGame(win_game_code* gameCode)
{
  if (gameCode->GameDLL) {
    FreeLibrary(gameCode->GameDLL);

    gameCode->GameDLL = NULL;
    gameCode->GameMain = GameMainStub;
    gameCode->GetSoundSamples = GameGetSoundSamplesStub;

    gameCode->Valid = false;
  }
}

static win_renderer_code LoadRenderer(const char* fileName)
{
  win_renderer_code loadRendererResult = {};

  char* tempDLLName = "ogl_main_loaded.dll";

  CopyFile(fileName, tempDLLName, FALSE);
  HMODULE rendererLibrary = LoadLibrary(tempDLLName);
  if (rendererLibrary) {
    loadRendererResult.RendererDLL = rendererLibrary;
    loadRendererResult.FileTimestamp = GetFileWriteTime(tempDLLName);
    loadRendererResult.RendererMain =
      (renderer_main*)GetProcAddress(rendererLibrary, "RendererMain");
    loadRendererResult.RendererLoadExtensions =
      (renderer_load_extensions*)GetProcAddress(rendererLibrary,
                                                "RendererLoadExtensions");

    loadRendererResult.Valid = (loadRendererResult.RendererMain &&
                                loadRendererResult.RendererLoadExtensions);
  }

  if (!loadRendererResult.Valid) {
    loadRendererResult.RendererMain = RendererMainStub;
    loadRendererResult.RendererLoadExtensions = RendererLoadExtensionsStub;
  }

  return loadRendererResult;
}

static void UnloadRenderer(win_renderer_code* rendererCode)
{
  if (rendererCode->RendererDLL) {
    FreeLibrary(rendererCode->RendererDLL);

    rendererCode->RendererDLL = NULL;
    rendererCode->RendererMain = RendererMainStub;

    rendererCode->Valid = false;
  }
}

void WinAddCursorInput(game_input_buffer& inputBuffer,
                       float deltaX,
                       float deltaY)
{
  if (GlobalPlaybackLoop.Playing) {
    return;
  }

  game_input newInputState = {};
  newInputState.State.Code = INPUT_CODE_CURSOR_MOVE;
  newInputState.Position.X = deltaX;
  newInputState.Position.Y = deltaY;
  inputBuffer.Inputs[inputBuffer.InputCount++] = newInputState;
}

void WinAddInput(game_input_buffer& inputBuffer,
                 game_input_code inputCode,
                 uint32_t flags)
{
  game_input newInputState = {};
  newInputState.State.Code = inputCode;
  newInputState.State.Flags = flags;
  inputBuffer.Inputs[inputBuffer.InputCount++] = newInputState;
}

#define X_INPUT_GET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(xinput_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
  return ERROR_DEVICE_NOT_CONNECTED;
}
static xinput_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name)                                                \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(xinput_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return ERROR_DEVICE_NOT_CONNECTED;
}
static xinput_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

static void LoadXInput()
{
  HMODULE xInputLibrary = LoadLibrary("xinput9_1_0.dll");
  if (!xInputLibrary) {
    xInputLibrary = LoadLibrary("xinput1_3.dll");
  } else {
    xInputLibrary = LoadLibrary("xinput1_4.dll");
    if (!xInputLibrary) {
      xInputLibrary = LoadLibrary("xinput1_3.dll");
    }
  }
  if (xInputLibrary) {
    XInputGetState =
      (xinput_get_state*)GetProcAddress(xInputLibrary, "XInputGetState");
    XInputSetState =
      (xinput_set_state*)GetProcAddress(xInputLibrary, "XInputSetState");
  }
}

static HRESULT SetupXAudio2(const win_sound_output winSoundOutput)
{
  HMODULE directSoundLibrary = LoadLibrary("XAUDIO2_9.DLL");

  IXAudio2* pXAudio2 = NULL;
  HRESULT hr;
  if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    return hr;

  IXAudio2MasteringVoice* pMasterVoice = NULL;
  if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice)))
    return hr;

  WAVEFORMATEX waveFormat = {};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = 2;
  waveFormat.nSamplesPerSec = winSoundOutput.SamplesPerSecond;
  waveFormat.wBitsPerSample = 16;
  waveFormat.nBlockAlign =
    (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
  waveFormat.nAvgBytesPerSec =
    waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;

  if (FAILED(hr = pXAudio2->CreateSourceVoice(&GlobalSourceVoice,
                                              (WAVEFORMATEX*)&waveFormat)))
    return hr;

  if (FAILED(hr = GlobalSourceVoice->Start(0)))
    return hr;

  return hr;
}

#define DIRECT_SOUND_CREATE(name)                                              \
  HRESULT WINAPI name(                                                         \
    LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

static void SetupDirectSound(HWND window, const win_sound_output winSoundOutput)
{
  HMODULE directSoundLibrary = LoadLibrary("dsound.dll");
  if (directSoundLibrary) {
    direct_sound_create* directSoundCreate =
      (direct_sound_create*)GetProcAddress(directSoundLibrary,
                                           "DirectSoundCreate");
    LPDIRECTSOUND directSoundObject;
    if (directSoundCreate &&
        SUCCEEDED(directSoundCreate(0, &directSoundObject, NULL))) {
      WAVEFORMATEX waveFormat = {};
      waveFormat.wFormatTag = WAVE_FORMAT_PCM;
      waveFormat.nChannels = 2;
      waveFormat.nSamplesPerSec = winSoundOutput.SamplesPerSecond;
      waveFormat.wBitsPerSample = 16;
      waveFormat.nBlockAlign =
        (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
      waveFormat.nAvgBytesPerSec =
        waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
      waveFormat.cbSize = 0;
      if (SUCCEEDED(
            directSoundObject->SetCooperativeLevel(window, DSSCL_PRIORITY))) {
        DSBUFFERDESC bufferDescription = {};
        bufferDescription.dwSize = sizeof(bufferDescription);
        bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
        LPDIRECTSOUNDBUFFER primaryBuffer;
        if (SUCCEEDED(directSoundObject->CreateSoundBuffer(
              &bufferDescription, &primaryBuffer, NULL))) {
          if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) {
          }
        }
      }

      DSBUFFERDESC bufferDescription = {};
      bufferDescription.dwSize = sizeof(bufferDescription);
      bufferDescription.dwFlags = 0;
      bufferDescription.dwBufferBytes = winSoundOutput.SecondaryBufferSize;
      bufferDescription.lpwfxFormat = &waveFormat;

      if (SUCCEEDED(directSoundObject->CreateSoundBuffer(
            &bufferDescription, &GlobalSoundBuffer, NULL))) {
      }
    }
  }
}

static void WinClearSoundBuffer(win_sound_output* platformSoundBuffer)
{
  VOID* soundRegion1;
  DWORD soundRegion1Size;
  VOID* soundRegion2;
  DWORD soundRegion2Size;
  if (SUCCEEDED(
        GlobalSoundBuffer->Lock(0,
                                platformSoundBuffer->SecondaryBufferSize,
                                &soundRegion1,
                                &soundRegion1Size,
                                &soundRegion2,
                                &soundRegion2Size,
                                NULL))) {
    uint8_t* destSample = (uint8_t*)soundRegion1;
    for (DWORD byteIndex = 0; byteIndex < soundRegion1Size; byteIndex++) {
      *destSample++ = 0;
    }

    destSample = (uint8_t*)soundRegion2;
    for (DWORD byteIndex = 0; byteIndex < soundRegion2Size; byteIndex++) {
      *destSample++ = 0;
    }
    GlobalSoundBuffer->Unlock(
      soundRegion1, soundRegion1Size, soundRegion2, soundRegion2Size);
  }
}

static void WinFillSoundBuffer(win_sound_output* platformSoundBuffer,
                               game_sound_output_buffer* gameSoundBuffer,
                               DWORD soundByteToLock,
                               DWORD soundBytesToWrite)
{
  VOID* soundRegion1;
  DWORD soundRegion1Size;
  VOID* soundRegion2;
  DWORD soundRegion2Size;
  if (SUCCEEDED(GlobalSoundBuffer->Lock(soundByteToLock,
                                        soundBytesToWrite,
                                        &soundRegion1,
                                        &soundRegion1Size,
                                        &soundRegion2,
                                        &soundRegion2Size,
                                        NULL))) {
    DWORD region1SampleCount =
      soundRegion1Size / platformSoundBuffer->BytesPerSample;
    int16_t* destSample = (int16_t*)soundRegion1;
    int16_t* sourceSample = gameSoundBuffer->SampleOut;
    for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount;
         sampleIndex++) {
      *destSample++ = *sourceSample++; // L
      *destSample++ = *sourceSample++; // R
      platformSoundBuffer->RunningSampleIndex++;
    }

    DWORD region2SampleCount =
      soundRegion2Size / platformSoundBuffer->BytesPerSample;
    destSample = (int16_t*)soundRegion2;
    for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount;
         sampleIndex++) {
      *destSample++ = *sourceSample++; // L
      *destSample++ = *sourceSample++; // R
      platformSoundBuffer->RunningSampleIndex++;
    }
    GlobalSoundBuffer->Unlock(
      soundRegion1, soundRegion1Size, soundRegion2, soundRegion2Size);
  }
}

static HRESULT XAudio2FillSoundBuffer(win_sound_output* platformSoundBuffer,
                                      game_sound_output_buffer* gameSoundBuffer)
{
  XAUDIO2_BUFFER xAudio2Buffer;
  xAudio2Buffer.Flags = 0;

  xAudio2Buffer.pAudioData = (BYTE*)gameSoundBuffer->SampleOut;
  xAudio2Buffer.AudioBytes =
    gameSoundBuffer->SampleCount * platformSoundBuffer->BytesPerSample;
  xAudio2Buffer.PlayBegin = 0;
  xAudio2Buffer.PlayLength = gameSoundBuffer->SampleCount;
  xAudio2Buffer.LoopBegin = 0;
  xAudio2Buffer.LoopLength = 0;
  xAudio2Buffer.LoopCount = 0;
  xAudio2Buffer.pContext = NULL;

  HRESULT hr;
  if (FAILED(hr = GlobalSourceVoice->SubmitSourceBuffer(&xAudio2Buffer)))
    return hr;

  platformSoundBuffer->RunningSampleIndex += gameSoundBuffer->SampleCount;

  return S_OK;
}

static win_window_dimensions GetWindowDimensions(const HWND window)
{
  win_window_dimensions dimensions;

  RECT rect;
  GetClientRect(window, &rect);

  dimensions.Width = rect.right;
  dimensions.Height = rect.bottom;

  return dimensions;
}

static void ResizeDIBSection(win_offscreen_buffer* buffer,
                             const int width,
                             const int height)
{
  if (buffer->Memory) {
    VirtualFree(buffer->Memory, NULL, MEM_RELEASE);
  }

  buffer->Width = width;
  buffer->Height = height;
  buffer->BytesPerPixel = 4;

  buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
  buffer->Info.bmiHeader.biWidth = buffer->Width;
  buffer->Info.bmiHeader.biHeight = -buffer->Height;
  buffer->Info.bmiHeader.biPlanes = 1;
  buffer->Info.bmiHeader.biBitCount = 32;
  buffer->Info.bmiHeader.biCompression = BI_RGB;

  int bitmapMemorySize = buffer->Width * buffer->Height * buffer->BytesPerPixel;
  buffer->Memory = VirtualAlloc(
    NULL, bitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  buffer->Pitch = buffer->Width * buffer->BytesPerPixel;
}

static void WinShowBuffer(const HDC hdc, const win_window_dimensions dimensions)
{
#if 0
    StretchDIBits(hdc,
        /*
        x, y, width, height,
        x, y, width, height,
        */
        0, 0, buffer->Width, buffer->Height,
        0, 0, dimensions.Width, dimensions.Height,
        buffer->Memory,
        &buffer->Info,
        DIB_RGB_COLORS, SRCCOPY);
    return;
#endif

  glViewport(0, 0, dimensions.Width, dimensions.Height);

  SwapBuffers(hdc);
}

static LRESULT CALLBACK WndProc(HWND windowHandle,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam)
{
  switch (message) {
    case WM_CLOSE:
      GameRunning = false;
      break;
    case WM_SIZE: {
      win_window_dimensions dimensions = GetWindowDimensions(windowHandle);
      // ResizeDIBSection(&GlobalBackbuffer, dimensions.Width,
      // dimensions.Height);

      game_window_info gameWindowInfo;
      gameWindowInfo.Width = dimensions.Width;
      gameWindowInfo.Height = dimensions.Height;
      GlobalRenderer.RendererMain(gameWindowInfo, RenderContext);
    } break;
    case WM_PAINT: {
      PAINTSTRUCT paint;
      HDC deviceContext = BeginPaint(windowHandle, &paint);

      win_window_dimensions dimensions = GetWindowDimensions(windowHandle);

      WinShowBuffer(deviceContext, dimensions);

      EndPaint(windowHandle, &paint);
    } break;
    case WM_ACTIVATEAPP: {
      if (!DEBUG_StickyWindow) {
        SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 255, LWA_ALPHA);
      } else {
        SetLayeredWindowAttributes(windowHandle, RGB(0, 0, 0), 200, LWA_ALPHA);
      }
    } break;
    case WM_QUIT:
      GameRunning = false;
      break;
    default:
      return DefWindowProc(windowHandle, message, wParam, lParam);
      break;
  }

  return 0;
}

PLATFORM_ADD_WORK_QUEUE_ENTRY(Win32PushJob)
{
  uint32_t nextIndex = (queue->WriteIndex + 1) % ARRAY_SIZE(queue->Jobs);
  HOKI_ASSERT(nextIndex != queue->ReadIndex);
  platform_work_queue_job* newJob = queue->Jobs + queue->WriteIndex;
  *newJob = {};
  newJob->Callback = callback;
  newJob->Data = data;
  queue->WorkAdded++;
  _WriteBarrier();
  queue->WriteIndex = nextIndex;
  ReleaseSemaphore(queue->SemaphoreHandle, 1, NULL);
}

bool Win32DoNextQueueWorkEntry(platform_work_queue* queue)
{
  bool queueHasWork = false;

  uint32_t currentIndex = queue->ReadIndex;
  uint32_t nextIndex = (currentIndex + 1) % ARRAY_SIZE(queue->Jobs);
  if (currentIndex != queue->WriteIndex) {
    uint32_t indexBeforeExchange =
      InterlockedCompareExchange(&queue->ReadIndex, nextIndex, currentIndex);

    if (indexBeforeExchange == currentIndex) {
      platform_work_queue_job job = queue->Jobs[indexBeforeExchange];
      job.Callback(job.Data);
      InterlockedIncrement(&queue->WorkCompleted);
    }

    queueHasWork = true;
  }

  return queueHasWork;
}

DWORD WinThreadProc(LPVOID lpParameter)
{
  platform_work_queue* queue = (platform_work_queue*)lpParameter;

  for (;;) {
#if 1
    if (!Win32DoNextQueueWorkEntry(queue)) {
      WaitForSingleObject(queue->SemaphoreHandle, 0);
    }
#else
    WaitForSingleObject(queue->SemaphoreHandle, 0);
#endif
  }

  //  return NULL;
}

PLATFORM_COMPLETE_ALL_QUEUE_WORK(Win32CompleteAllWork)
{
  while (queue->WorkAdded != queue->WorkCompleted) {
    Win32DoNextQueueWorkEntry(queue);
  }

  int workAdded = queue->WorkAdded;
  int workCompleted = queue->WorkCompleted;
  HOKI_ASSERT(workAdded == workCompleted);

  queue->WorkAdded = 0;
  queue->WorkCompleted = 0;
}

platform_work_queue GlobalWorkQueue = {};

int CALLBACK WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
  LoadXInput();

  GlobalRenderer.RendererMain = RendererMainStub;

  HANDLE semaphoreHandle = CreateSemaphore(0, 0, 8, NULL);
  GlobalWorkQueue.SemaphoreHandle = semaphoreHandle;

  for (int i = 0; i < 8; i++) {
    HANDLE threadHandle =
      CreateThread(0, 0, WinThreadProc, &GlobalWorkQueue, 0, NULL);
  }

  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = WINDOW_CLASS;
  wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&wcex)) {
    MessageBox(
      NULL, _T("Call to RegisterClassEx failed!"), _T(WINDOW_TITLE), NULL);

    return 1;
  }

  HWND windowHandle = CreateWindowExA(WS_EX_LAYERED, // | WS_EX_TOPMOST,
                                      WINDOW_CLASS,
                                      WINDOW_TITLE,
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      WINDOW_WIDTH,
                                      WINDOW_HEIGHT,
                                      NULL,
                                      NULL,
                                      hInstance,
                                      NULL);

  if (!windowHandle) {
    MessageBox(
      NULL, _T("Call to CreateWindow failed!"), _T(WINDOW_TITLE), NULL);

    return 1;
  }

  UINT desiredSchedulerMs = 1;
  bool schedulerMsSet = timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR;

  int monitorHz = 60;
  int gameUpdateHz = monitorHz;
  float targetSecondsPerFrame = 1.0f / gameUpdateHz;
  int bytesPerFrame = (int)gameUpdateHz;

  MSG msg;

  static win_sound_output winSoundOutput = {};
  winSoundOutput.SamplesPerSecond = 48000;
  winSoundOutput.BytesPerSample = sizeof(int16_t) * 2;
  winSoundOutput.SecondaryBufferSize =
    (winSoundOutput.SamplesPerSecond * winSoundOutput.BytesPerSample);
  winSoundOutput.RunningSampleIndex = 0;
  winSoundOutput.SafetyMarginBytes =
    (winSoundOutput.SamplesPerSecond * winSoundOutput.BytesPerSample /
     gameUpdateHz) /
    2;

  if (FAILED(SetupXAudio2(winSoundOutput))) {
    SetupDirectSound(windowHandle, winSoundOutput);
  }
  // Todo: MEM_LARGE_PAGES
  int16_t* Samples = (int16_t*)VirtualAlloc(NULL,
                                            winSoundOutput.SecondaryBufferSize,
                                            MEM_RESERVE | MEM_COMMIT,
                                            PAGE_READWRITE);
  InputBuffer.Inputs =
    (game_input*)VirtualAlloc(NULL,
                              sizeof(game_input) * INPUT_BUFFER_SIZE,
                              MEM_RESERVE | MEM_COMMIT,
                              PAGE_READWRITE);

#if HOKI_DEV
  LPVOID memoryBase = (LPVOID)SIZE_GB((uint64_t)2);
#else
  LPVOID memoryBase = 0;
#endif

  game_memory Memory = {};
  Memory.PermanentStorageSize = SIZE_MB(64);
  Memory.TransientStorageSize = SIZE_MB(128);
  uint64_t totalSize =
    Memory.PermanentStorageSize + Memory.TransientStorageSize;
  Memory.PermanentStorage = VirtualAlloc(
    memoryBase, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  Memory.TransientStorage =
    ((uint8_t*)Memory.PermanentStorage + Memory.PermanentStorageSize);

  Memory.ReadFile = WinReadFile;
  Memory.GetFileSize = WinGetFileSize;
  Memory.WriteFile = WinWriteFile;
  Memory.Log = WinLog;
  Memory.AddWorkEntry = Win32PushJob;
  Memory.CompleteAllQueueWork = Win32CompleteAllWork;
  Memory.WorkQueue = &GlobalWorkQueue;

  GlobalPlaybackLoop = {};
  GlobalPlaybackLoop.Memory = Memory.PermanentStorage;
  GlobalPlaybackLoop.MemorySize = Memory.PermanentStorageSize;

  if (GlobalSoundBuffer != NULL) {
    WinClearSoundBuffer(&winSoundOutput);
    GlobalSoundBuffer->Play(NULL, NULL, DSBPLAY_LOOPING);
  }

  DWORD lastFrameTime = WinGetTime();

  char* gameDLLName = "game_main.dll";
  win_game_code gameCode = LoadGame(gameDLLName);
  FILETIME gameDLLLastWrite = GetFileWriteTime(gameDLLName);

  char* rendererDLLName = "ogl_main.dll";
  GlobalRenderer = LoadRenderer(rendererDLLName);
  FILETIME rendererDLLLastWrite = GetFileWriteTime(rendererDLLName);

  // Create a temporary window to work around
  // opengl + windows issues when choosing a pixel format

  HWND tempWindow = CreateWindowExA(WS_EX_LAYERED,
                                    WINDOW_CLASS,
                                    WINDOW_TITLE,
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    WINDOW_WIDTH,
                                    WINDOW_HEIGHT,
                                    NULL,
                                    NULL,
                                    hInstance,
                                    NULL);

  if (!tempWindow) {
    MessageBox(
      NULL, _T("Call to CreateWindow failed!"), _T(WINDOW_TITLE), NULL);

    return 1;
  }
  WinSetupOpenGL(windowHandle, tempWindow, GlobalRenderer);

  float secElapsedForFrame = 0.0f;

  while (GameRunning) {
#if HOKI_DEV
    gameDLLLastWrite = GetFileWriteTime(gameDLLName);
    if (CompareFileTime(&gameDLLLastWrite, &gameCode.FileTimestamp) != 0) {
      UnloadGame(&gameCode);
      gameCode = LoadGame(gameDLLName);
    }

    rendererDLLLastWrite = GetFileWriteTime(rendererDLLName);
    if (CompareFileTime(&rendererDLLLastWrite, &GlobalRenderer.FileTimestamp) !=
        0) {
      UnloadRenderer(&GlobalRenderer);
      GlobalRenderer = LoadRenderer(rendererDLLName);
      GlobalRenderer.RendererLoadExtensions();
    }
#endif
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      bool mouseDown = false;
      switch (msg.message) {
        case WM_LBUTTONDOWN:
          mouseDown = true;
          WinAddInput(InputBuffer,
                      INPUT_CODE_CURSOR_CLICK,
                      INPUT_STATE_CHANGED | INPUT_STATE_IS_DOWN);
          break;
        case WM_LBUTTONUP:
          mouseDown = false;
          WinAddInput(InputBuffer,
                      INPUT_CODE_CURSOR_CLICK,
                      INPUT_STATE_CHANGED | INPUT_STATE_IS_UP);
          break;
        case WM_RBUTTONDOWN:
          mouseLocked = !mouseLocked;
          ShowCursor(!mouseLocked);
          break;

        case WM_MOUSEMOVE: {
          if (mouseLocked) {
            win_window_dimensions dimensions =
              GetWindowDimensions(windowHandle);
            POINTS mousePoints = MAKEPOINTS(msg.lParam);
            float mouseX = mousePoints.x == 0
                             ? -0.5f
                             : (float)mousePoints.x / dimensions.Width - 0.5f;
            float mouseY = mousePoints.y == 0
                             ? -0.5f
                             : (float)mousePoints.y / dimensions.Height - 0.5f;

            mouseX = fabs(mouseX) > 0.001f ? mouseX : 0.0f;
            mouseY = fabs(mouseY) > 0.001f ? mouseY : 0.0f;

            WinAddCursorInput(InputBuffer, mouseX, mouseY);

            POINT centerPoint = { dimensions.Width / 2, dimensions.Height / 2 };
            ClientToScreen(windowHandle, &centerPoint);
            SetCursorPos(centerPoint.x, centerPoint.y);
          }
        } break;

        case WM_SYSKEYDOWN:
          break;
        case WM_SYSKEYUP:
          break;
        case WM_KEYDOWN:
        case WM_KEYUP: {
          bool altKeyDown = (msg.lParam & (1 << 29)) != 0;
          bool wasDown = msg.lParam & 0x40000000;
          bool isDown = (msg.lParam & (1 << 31)) == 0;

          float factor = 1.0f;
          uint32_t inputFlags = INPUT_STATE_NO_STATE;
          inputFlags |= isDown ? INPUT_STATE_IS_DOWN : INPUT_STATE_IS_UP;
          inputFlags |= (wasDown != isDown) ? INPUT_STATE_CHANGED : 0x0;

          bool validInput = true;
          game_input_code code = INPUT_CODE_NO_OP;

          uint32_t vkCode = (uint32_t)msg.wParam;
          switch (vkCode) {
            case VK_UP:
            case 'W':
              code = INPUT_CODE_UP;
              break;
            case VK_DOWN:
            case 'S':
              code = INPUT_CODE_DOWN;
              break;
            case VK_LEFT:
            case 'A':
              code = INPUT_CODE_LEFT;
              break;
            case VK_RIGHT:
            case 'D':
              code = INPUT_CODE_RIGHT;
              break;

            case '1':
              code = INPUT_CODE_NUM_1;
              break;
            case '2':
              code = INPUT_CODE_NUM_2;
              break;

            case VK_LBUTTON:
            case '3':
              code = INPUT_CODE_NUM_3;
              break;

            case VK_RBUTTON:
            case '4':
              code = INPUT_CODE_NUM_4;
              break;

            case '5':
              code = INPUT_CODE_NUM_5;
              break;

            case VK_F4:
              if (!altKeyDown) {
                break;
              }
            case VK_ESCAPE:
              GameRunning = false;
              break;

            case 'R': {
              code = INPUT_CODE_R;
              if (isDown && !wasDown) {
                if (!GlobalPlaybackLoop.Recording &&
                    !GlobalPlaybackLoop.Playing) {
                  SetWindowTextA(windowHandle, "RECORDING");

                  WinPlaybackStartRecord(&GlobalPlaybackLoop);
                } else if (GlobalPlaybackLoop.Recording) {
                  SetWindowTextA(windowHandle, "PLAYING");
                  WinPlaybackStopRecord(&GlobalPlaybackLoop);
                  WinPlaybackStartPlayback(&GlobalPlaybackLoop);
                } else if (GlobalPlaybackLoop.Playing) {
                  SetWindowTextA(windowHandle, "STOPPED");
                  WinPlaybackStopPlayback(&GlobalPlaybackLoop);
                }
              }
              validInput = false;
            } break;

            case 'Q': {
              code = INPUT_CODE_R;
              if (isDown && !wasDown) {
                if (!GlobalPlaybackLoop.Playing) {
                  SetWindowTextA(windowHandle, "PLAYING");
                  WinPlaybackStartPlayback(&GlobalPlaybackLoop);
                } else if (GlobalPlaybackLoop.Playing) {
                  SetWindowTextA(windowHandle, "STOPPED");
                  WinPlaybackStopPlayback(&GlobalPlaybackLoop);
                }
              }
              validInput = false;
            } break;

            case VK_SPACE: {
              if (inputFlags & INPUT_STATE_IS_UP) {
                DEBUG_StickyWindow = !DEBUG_StickyWindow;
                HWND topmostStatus =
                  DEBUG_StickyWindow ? HWND_TOPMOST : HWND_BOTTOM;
                SetWindowPos(windowHandle,
                             topmostStatus,
                             0,
                             0,
                             0,
                             0,
                             SWP_NOMOVE | SWP_NOSIZE);
              }
            } break;

            default:
              validInput = false;
              break;
          }

          if (validInput) {
            WinAddInput(InputBuffer, code, inputFlags);
          }
          break;
        }

        default: {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        } break;
      }
    }
    for (int controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT;
         controllerIndex++) {
      XINPUT_STATE controllerState;
      if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) {
        XINPUT_GAMEPAD* gamePad = &controllerState.Gamepad;

        bool up = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
        bool down = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
        bool left = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
        bool right = (gamePad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

        int x = (left ? 0 : -1) + (right ? 0 : 1);
        int y = (up ? 0 : -1) + (down ? 0 : 1);

        WinAddCursorInput(InputBuffer, (float)x, (float)y);
      } else {
        // Controller disconnected
      }
    }

    POINT centerPoint = { 0, 0 };
    ClientToScreen(windowHandle, &centerPoint);
    game_window_info gameWindowInfo;
    win_window_dimensions dimensions = GetWindowDimensions(windowHandle);
    gameWindowInfo.Width = dimensions.Width;
    gameWindowInfo.Height = dimensions.Height;

    if (GlobalPlaybackLoop.Recording) {
      WinPlaybackLoopRecordInput(
        &GlobalPlaybackLoop, &InputBuffer, secElapsedForFrame);
    } else if (GlobalPlaybackLoop.Playing) {
      WinPlaybackLoopPlayInput(
        &GlobalPlaybackLoop, &InputBuffer, secElapsedForFrame);
    }

    RenderContext.Commands.Count = 0;
    gameCode.GameMain(
      Memory, gameWindowInfo, RenderContext, InputBuffer, secElapsedForFrame);
    GlobalRenderer.RendererMain(gameWindowInfo, RenderContext);

    HDC deviceContext = GetDC(windowHandle);
    WinShowBuffer(deviceContext, dimensions);
    ReleaseDC(windowHandle, deviceContext);

    DWORD playCursor = 0;
    DWORD writeCursor = 0;
    if (GlobalSoundBuffer != NULL && GlobalSoundBuffer->GetCurrentPosition(
                                       &playCursor, &writeCursor) == DS_OK) {
      /*
          Here is how sound output computation works:
          We define a saftey value that is the number of samples that
          we think our game update loop can vary by (like 2 ms).
          When we wake up to write audio, we will look and see what the
          play cursor position is, and we will forecast
          ahead where we think the play cursor will be on
          the next frame boundary.
          We will then look to see if the write cursor
          is before that by at least our saftey value.
          If it is, the target frame position is that frame boundary plus
          one frame. This gives us perfect audio sync in the case of a card
          that has low latency.
          If the write cursor is after the safety margin,
          then we assume we can never sync the audio perfectly, so we will
          write one frame's worth of audio plus the safety maring's
          worth of samples.
      */

      // Writing from byteToLock to targetCursor.
      // byteToLock is going to be wherever we last wrote to.
      DWORD byteToLock =
        (winSoundOutput.RunningSampleIndex * winSoundOutput.BytesPerSample) %
        winSoundOutput.SecondaryBufferSize;

      DWORD expectedSoundBytesPerFrame =
        (winSoundOutput.SamplesPerSecond * winSoundOutput.BytesPerSample) /
        gameUpdateHz;

      DWORD expectedFrameBoundaryByte = playCursor + expectedSoundBytesPerFrame;

      DWORD safeWriteCursor =
        writeCursor; // safwWriteCursor accounts for the variability of the
                     // queried writeCursor
      if (safeWriteCursor < playCursor) {
        safeWriteCursor += winSoundOutput.SecondaryBufferSize;
      }
      safeWriteCursor += winSoundOutput.SafetyMarginBytes;
      bool audioCardIsLowLatency =
        (safeWriteCursor < expectedFrameBoundaryByte); // writecursor is inside
                                                       // of the current frame

      DWORD targetCursor = 0;
      if (audioCardIsLowLatency) {
        targetCursor = (expectedFrameBoundaryByte + expectedSoundBytesPerFrame);
      } else {
        targetCursor = (writeCursor + expectedSoundBytesPerFrame +
                        winSoundOutput.SafetyMarginBytes);
      }
      targetCursor %= winSoundOutput.SecondaryBufferSize;

      DWORD bytesToWrite = 0;
      if (byteToLock > targetCursor) {
        bytesToWrite = winSoundOutput.SecondaryBufferSize - byteToLock;
        bytesToWrite += targetCursor;
      } else {
        bytesToWrite = targetCursor - byteToLock;
      }

      game_sound_output_buffer gameSoundBuffer = {};
      gameSoundBuffer.SamplesPerSecond = winSoundOutput.SamplesPerSecond;
      gameSoundBuffer.SampleCount =
        (bytesToWrite / winSoundOutput.BytesPerSample);
      gameSoundBuffer.SampleOut = Samples;

      gameCode.GetSoundSamples(&Memory, &gameSoundBuffer);
      WinFillSoundBuffer(
        &winSoundOutput, &gameSoundBuffer, byteToLock, bytesToWrite);
    } else {
      XAUDIO2_VOICE_STATE pVoiceState;
      GlobalSourceVoice->GetState(&pVoiceState, 0);

      if (pVoiceState.BuffersQueued < 2) {
        DWORD bufferSizeSamples =
          winSoundOutput.SecondaryBufferSize / winSoundOutput.BytesPerSample;
        DWORD samplesWritten =
          winSoundOutput.RunningSampleIndex % bufferSizeSamples;
        DWORD expectedSamplesPerFrame =
          (winSoundOutput.SamplesPerSecond / gameUpdateHz);

        game_sound_output_buffer gameSoundBuffer = {};
        gameSoundBuffer.SamplesPerSecond = winSoundOutput.SamplesPerSecond;
        gameSoundBuffer.SampleCount = expectedSamplesPerFrame * 3;
        DWORD* samplesOffset = (DWORD*)Samples + samplesWritten;
        gameSoundBuffer.SampleOut = (int16_t*)samplesOffset;

        // Split buffer for any overwritten samples
        if (samplesWritten + gameSoundBuffer.SampleCount > bufferSizeSamples) {
          DWORD samplesOver =
            (samplesWritten + gameSoundBuffer.SampleCount) - bufferSizeSamples;

          game_sound_output_buffer splitGameSoundBuffer = {};
          splitGameSoundBuffer.SamplesPerSecond =
            winSoundOutput.SamplesPerSecond;
          splitGameSoundBuffer.SampleCount = samplesOver;
          splitGameSoundBuffer.SampleOut = Samples;

          gameSoundBuffer.SampleCount -= samplesOver;

          gameCode.GetSoundSamples(&Memory, &gameSoundBuffer);
          XAudio2FillSoundBuffer(&winSoundOutput, &gameSoundBuffer);
          if (samplesOver > 0) {
            gameCode.GetSoundSamples(&Memory, &splitGameSoundBuffer);
            XAudio2FillSoundBuffer(&winSoundOutput, &splitGameSoundBuffer);
          }
        } else {
          gameCode.GetSoundSamples(&Memory, &gameSoundBuffer);
          XAudio2FillSoundBuffer(&winSoundOutput, &gameSoundBuffer);
        }
      }
    }

    DWORD frameEndTime = WinGetTime();
    secElapsedForFrame = WinGetElapsedSeconds(lastFrameTime, frameEndTime);
    DWORD sleepForMs = 0;
    if (secElapsedForFrame < targetSecondsPerFrame) {
      if (schedulerMsSet) {
        sleepForMs =
          (DWORD)(1000.0f * (targetSecondsPerFrame - secElapsedForFrame));
        Sleep(sleepForMs);
      }
      secElapsedForFrame = WinGetElapsedSeconds(lastFrameTime, WinGetTime());
      while (secElapsedForFrame < targetSecondsPerFrame) {
        secElapsedForFrame = WinGetElapsedSeconds(lastFrameTime, WinGetTime());
      }
    }

#ifdef HOKI_DEV
    secElapsedForFrame = min_f(targetSecondsPerFrame, secElapsedForFrame);
    if (secElapsedForFrame > 1.0f) {
      // Cap the elapsed seconds in case we are in a breakpoint
      secElapsedForFrame = targetSecondsPerFrame;
    }
#endif
    lastFrameTime = WinGetTime();
#if 0
        int64_t fps = (int)(1000.0f * WinGetElapsedSeconds(lastFrameTime, WinGetTime()));
        float msPerFrame = 1000.0f * WinGetElapsedSeconds(lastFrameTime, frameEndTime);
        char buffer[256];
        sprintf_s(buffer, sizeof(buffer), "%.2fms\n", (1000.0f * secElapsedForFrame));
        OutputDebugStringA(buffer);
#endif
  }

  return 0;
}
