
static PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2D;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
static PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
static PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERARBPROC glCompileShader;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLGETPROGRAMIVPROC glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLDELETESHADERPROC glDeleteShader;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
static PFNGLBUFFERSUBDATAPROC glBufferSubData;
static PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;

static PFNGLUNIFORM1FPROC glUniform1f;
static PFNGLUNIFORM2FPROC glUniform2f;
static PFNGLUNIFORM3FPROC glUniform3f;
static PFNGLUNIFORM4FPROC glUniform4f;

static PFNGLUNIFORM1IPROC glUniform1i;
static PFNGLUNIFORM2IPROC glUniform2i;
static PFNGLUNIFORM3IPROC glUniform3i;
static PFNGLUNIFORM4IPROC glUniform4i;

static PFNGLUNIFORM1FVPROC glUniform1fv;
static PFNGLUNIFORM2FVPROC glUniform2fv;
static PFNGLUNIFORM3FVPROC glUniform3fv;
static PFNGLUNIFORM4FVPROC glUniform4fv;

static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
static PFNGLACTIVETEXTUREPROC glActiveTexture;
static PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

static PFNGLOBJECTLABELPROC glObjectLabel;
static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC
  glGetFramebufferAttachmentParameteriv;

extern "C" RENDERER_LOAD_EXTENSIONS(RendererLoadExtensions)
{
  glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)wglGetProcAddress(
    "glCompressedTexImage2DARB");
  if (glCompressedTexImage2D == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
  if (glGenBuffers == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
  if (glBindBuffer == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
  if (glBufferData == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glVertexAttribPointer =
    (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
  if (glVertexAttribPointer == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glVertexAttribIPointer =
    (PFNGLVERTEXATTRIBIPOINTERPROC)wglGetProcAddress("glVertexAttribIPointer");
  if (glVertexAttribIPointer == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }
  glEnableVertexAttribArray =
    (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress(
      "glEnableVertexAttribArray");
  if (glEnableVertexAttribArray == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glDisableVertexAttribArray =
    (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress(
      "glDisableVertexAttribArray");
  if (glDisableVertexAttribArray == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glVertexAttrib3f =
    (PFNGLVERTEXATTRIB3FPROC)wglGetProcAddress("glVertexAttrib3f");
  if (glVertexAttrib3f == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
  if (glCreateShader == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
  if (glShaderSource == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glCompileShader =
    (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShader");
  if (glCompileShader == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
  if (glGetShaderiv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGetShaderInfoLog =
    (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
  if (glGetShaderInfoLog == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glCreateProgram =
    (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
  if (glCreateProgram == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
  if (glUseProgram == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
  if (glAttachShader == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
  if (glLinkProgram == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
  if (glGetProgramiv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGetProgramInfoLog =
    (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
  if (glGetProgramInfoLog == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
  if (glDeleteShader == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGenVertexArrays =
    (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
  if (glGenVertexArrays == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glBindVertexArray =
    (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
  if (glBindVertexArray == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGetUniformLocation =
    (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
  if (glGetUniformLocation == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glBufferSubData =
    (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
  if (glBufferSubData == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)wglGetProcAddress(
    "glDrawElementsInstanced");
  if (glDrawElementsInstanced == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
  if (glUniform1f == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
  if (glUniform2f == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
  if (glUniform3f == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
  if (glUniform4f == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
  if (glUniform1i == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform2i = (PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i");
  if (glUniform2i == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform3i = (PFNGLUNIFORM3IPROC)wglGetProcAddress("glUniform3i");
  if (glUniform3i == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform4i = (PFNGLUNIFORM4IPROC)wglGetProcAddress("glUniform4i");
  if (glUniform4i == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
  if (glUniform1fv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform2fv = (PFNGLUNIFORM2FVPROC)wglGetProcAddress("glUniform2fv");
  if (glUniform2fv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
  if (glUniform3fv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
  if (glUniform4fv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glUniformMatrix4fv =
    (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
  if (glUniformMatrix4fv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glActiveTexture =
    (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
  if (glActiveTexture == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGenerateMipmap =
    (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
  if (glGenerateMipmap == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGetFramebufferAttachmentParameteriv =
    (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress(
      "glGetFramebufferAttachmentParameteriv");
  if (glGetFramebufferAttachmentParameteriv == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glGenFramebuffers =
    (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
  if (glGenFramebuffers == NULL) {
    glGenFramebuffers =
      (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffersEXT");
  }
  if (glGenFramebuffers == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glBindFramebuffer =
    (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebuffer");
  if (glBindFramebuffer == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glFramebufferTexture2D =
    (PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress(
      "glFramebufferTexture2D");
  if (glFramebufferTexture2D == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress(
    "glCheckFramebufferStatus");
  if (glCheckFramebufferStatus == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  glObjectLabel = (PFNGLOBJECTLABELPROC)wglGetProcAddress("glObjectLabel");
  if (glObjectLabel == NULL) {
    return HOKI_OGL_EXTENSIONS_FAILED;
  }

  return HOKI_OGL_EXTENSIONS_OK;
}