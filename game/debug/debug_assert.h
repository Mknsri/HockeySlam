#ifndef HOKI_DEBUG_ASSERT
#define HOKI_DEBUG_ASSERT

#if HOKI_DEV
#ifdef _WIN32
#include <intrin.h>
#define DBG_BREAK __debugbreak()
#elif __ANDROID__
#define DBG_BREAK raise(SIGTRAP);
#endif
#include <cstdarg>
#include <stdio.h>

void noOpToFixInstructionPointer()
{
  int i = 0;
}

void __Hoki_Assert(const char* expr_str,
                   bool expr,
                   const char* func,
                   const char* file,
                   int line,
                   const char* msg,
                   ...)
{
  if (!expr) {
    char buffer[1024 * 2] = {};
    char message[1024] = {};

    va_list argptr;
    va_start(argptr, msg);
    size_t strLen = vsnprintf(message, sizeof(message), msg, argptr);
    va_end(argptr);

#if 0
    snprintf(buffer,
             sizeof(buffer),
#else
    DEBUG_LOG(
#endif
             "Assert failed: %s\n"
             "Expected:\t%s\n"
             "Function:\t%s\n"
             "Source:\t\t%s, line %d\n",
             message,
             expr_str,
             func,
             file,
             line);
             DBG_BREAK;
  }
}

void __Hoki_Warn(const char* expr_str,
                 bool expr,
                 const char* func,
                 const char* file,
                 int line,
                 const char* msg,
                 ...)
{
  if (!expr) {
    char buffer[1024] = {};
    char message[1024] = {};

    va_list argptr;
    va_start(argptr, msg);
    size_t strLen = vsnprintf(message, sizeof(message), msg, argptr);
    va_end(argptr);

#if 0
    snprintf(buffer,
             sizeof(buffer),
#else
    DEBUG_LOG(
#endif
             "Warning: %s\n"
             "Expected:\t%s\n"
             "Function:\t%s\n"
             "Source:\t\t%s, line %d\n",
             message,
             expr_str,
             func,
             file,
             line);
  }
}

#define HOKI_WARN(Expr)                                                        \
  __Hoki_Warn(#Expr, Expr, __func__, __FILE__, __LINE__, #Expr, 0);            \
  noOpToFixInstructionPointer()

#define HOKI_WARN_MESSAGE(Expr, Msg, ...)                                      \
  __Hoki_Warn(#Expr, Expr, __func__, __FILE__, __LINE__, Msg, ##__VA_ARGS__);  \
  noOpToFixInstructionPointer()

#define HOKI_ASSERT(Expr)                                                      \
  __Hoki_Assert(#Expr, Expr, __func__, __FILE__, __LINE__, #Expr, 0);          \
  noOpToFixInstructionPointer()

#define HOKI_ASSERT_MESSAGE(Expr, Msg, ...)                                    \
  __Hoki_Assert(                                                               \
    #Expr, Expr, __func__, __FILE__, __LINE__, Msg, ##__VA_ARGS__);            \
  noOpToFixInstructionPointer()

#define HOKI_ASSERT_NO_OPENGL_ERRORS()                                         \
  do {                                                                         \
    GLenum error = glGetError();                                               \
    HOKI_ASSERT(error == GL_NO_ERROR);                                         \
  } while (false)

#else
#define HOKI_WARN(Expr)
#define HOKI_WARN_MESSAGE(Expr, Msg, ...)
#define HOKI_ASSERT(Expr)
#define HOKI_ASSERT_MESSAGE(Expr, Msg, ...)
#define HOKI_ASSERT_NO_OPENGL_ERRORS()
#endif

#endif // HOKI_DEBUG_ASSERT