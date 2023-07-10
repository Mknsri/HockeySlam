#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#define PLATFORM_LOG(name) void name(const char* fmt, ...)
typedef PLATFORM_LOG(platform_log);
PLATFORM_LOG(PlatformLogStub) {}

static platform_log* DEBUG_LOG = PlatformLogStub;

#endif