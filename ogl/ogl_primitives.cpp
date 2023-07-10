static uint32_t BindPoint(render_context& context)
{
  if (context.PointPrimitive != HOKI_OGL_NO_ID) {
    return context.PointPrimitive;
  }

  uint32_t VAO, VBO;

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  float kek[] = { 0.0f, 0.0f, 0.0f };
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kek), &kek[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  context.PointPrimitive = VAO;
  return context.PointPrimitive;
}

static uint32_t BindLine(render_context& context)
{
  if (context.LinePrimitive != HOKI_OGL_NO_ID) {
    return context.LinePrimitive;
  }

  uint32_t VAO, VBO;

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  float kek[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kek), &kek[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  context.LinePrimitive = VAO;
  return context.LinePrimitive;
}

static void DEBUG_RenderLine(const debug_render_line& line,
                             render_context& context,
                             v3 lineColor)
{
  GLint lineId = BindLine(context);
#ifndef GL_ES_VERSION_3_0
  glEnable(GL_LINE_SMOOTH);
#endif

  mat4x4 modelMatrix = IDENTITY_MATRIX * mat4x4_translate(line.Position) *
                       mat4x4_scale(line.Length);
  mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;

  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.SimpleShader.ViewProjection, &viewProjection, 1);
  SetUniform(context.SimpleShader.ObjectColor, &lineColor, 1);

  // draw mesh
  glBindVertexArray(lineId);
  glDrawArrays(GL_LINES, 0, 2);
  glBindVertexArray(0);
#ifndef GL_ES_VERSION_3_0
  glDisable(GL_LINE_SMOOTH);
#endif
}

static void DEBUG_RenderLine(const debug_render_line& line,
                             render_context& context)
{
  DEBUG_RenderLine(line, context, _v3(1.0f, 0.0f, 1.0f));
}

static uint32_t BindRect(render_context& context)
{
  if (context.RectPrimitive != HOKI_OGL_NO_ID) {
    return context.RectPrimitive;
  }

  GLuint VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  const GLfloat vertices[6][4] = { { 0.0f, 0.0f, 0.0, 0.0 },
                                   { 0.0f, 1.0f, 0.0, 1.0 },
                                   { 1.0f, 0.0f, 1.0, 0.0 },

                                   { 0.0f, 1.0f, 0.0, 1.0 },
                                   { 1.0f, 0.0f, 1.0, 0.0 },
                                   { 1.0f, 1.0f, 1.0, 1.0 } };

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);

  // vertex texture coords
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(GLfloat) * 4,
                        (void*)(sizeof(GLfloat) * 2));

  context.RectPrimitive = VAO;
  return context.RectPrimitive;
}

static uint32_t BindCube(render_context& context)
{
  if (context.CubePrimitive != HOKI_OGL_NO_ID) {
    return context.CubePrimitive;
  }

  uint32_t VAO, VBO;

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  float kek[] = {
    -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
    0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

    -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

    0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
    0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
    0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

    -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f
  };
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kek), &kek[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  context.CubePrimitive = VAO;
  return context.CubePrimitive;
}

static uint32_t BindSphere(render_context& context)
{
  if (context.SpherePrimitive != HOKI_OGL_NO_ID) {
    return context.SpherePrimitive;
  }

  uint32_t VAO, VBO;

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  float scale = 0.5f;
  const int pointsPerCircle = 30;
  v3 kek[pointsPerCircle * 3];

  int offset = 0;
  float fourPi = (float)M_2PI * 2;
  for (int i = 0; i < pointsPerCircle; i++) {
    float seg = ((float)i / pointsPerCircle) * fourPi;
    kek[i + offset].X = (float)std::sin((seg)) * scale;
    kek[i + offset].Y = (float)std::cos((seg)) * scale;
    kek[i + offset].Z = 0.0f;
  }

  offset = pointsPerCircle;
  for (int i = 0; i < pointsPerCircle; i++) {
    float seg = ((float)i / pointsPerCircle) * fourPi;
    kek[i + offset].X = 0.0f;
    kek[i + offset].Y = (float)std::sin(seg) * scale;
    kek[i + offset].Z = (float)std::cos(seg) * scale;
  }

  offset = pointsPerCircle + pointsPerCircle;
  for (int i = 0; i < pointsPerCircle; i++) {
    float seg = ((float)i / pointsPerCircle) * fourPi;
    kek[i + offset].X = (float)std::sin(seg) * scale;
    kek[i + offset].Y = 0.0f;
    kek[i + offset].Z = (float)std::cos(seg) * scale;
  }

  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kek), &kek[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  context.SpherePrimitive = context.SpherePrimitive;

  return context.SpherePrimitive;
}

static uint32_t BindTetrahedron(render_context& context)
{
  if (context.TetrahedronPrimitive != HOKI_OGL_NO_ID) {
    return context.TetrahedronPrimitive;
  }

  uint32_t VAO, VBO;

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  float kek[] = { 0.5f,  0.0f, 0.5f, -0.5f, 0.0f, 0.5f,  0.0f,  1.0f, 0.0f,

                  0.5f,  0.0f, 0.5f, 0.0f,  0.0f, -0.5f, 0.0f,  1.0f, 0.0f,

                  -0.5f, 0.0f, 0.5f, 0.0f,  0.0f, -0.5f, 0.0f,  1.0f, 0.0f,

                  0.5f,  0.0f, 0.5f, 0.0f,  0.0f, -0.5f, -0.5f, 0.0f, 0.5f };
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kek), &kek[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  context.TetrahedronPrimitive = VAO;
  return context.TetrahedronPrimitive;
}

static uint32_t BindPyramid(render_context& context)
{
  if (context.PyramidPrimitive != HOKI_OGL_NO_ID) {
    return context.PyramidPrimitive;
  }

  uint32_t VAO, VBO;

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  float kek[] = {
    0.25f,  0.0f, 0.25f,  -0.25f, 0.0f, 0.25f,  0.0f,  0.75f, 0.0f,

    0.25f,  0.0f, 0.25f,  0.25f,  0.0f, -0.25f, 0.0f,  0.75f, 0.0f,

    0.25f,  0.0f, -0.25f, -0.25f, 0.0f, -0.25f, 0.0f,  0.75f, 0.0f,

    -0.25f, 0.0f, 0.25f,  -0.25f, 0.0f, -0.25f, -0.0f, 0.75f, 0.0f,

    0.25f,  0.0f, 0.25f,  0.25f,  0.0f, -0.25f, 0.0f,  0.0f,  0.0f,

    -0.25f, 0.0f, 0.25f,  -0.25f, 0.0f, -0.25f, 0.0f,  0.0f,  0.0f
  };
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kek), &kek[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);

  context.PyramidPrimitive = VAO;
  return context.PyramidPrimitive;
}

static void DEBUG_RenderFrustum(const mat4x4& projectionMatrix,
                                render_context& context,
                                const v3 color)
{
  float frustumScale = 1.0f;
  mat4x4 projInv = mat4x4_invert(projectionMatrix);
  v4 RTN = projInv * _v4(frustumScale, frustumScale, -frustumScale, 1.0f);
  RTN = RTN / RTN.W;
  v4 LTN = projInv * _v4(-frustumScale, frustumScale, -frustumScale, 1.0f);
  LTN = LTN / LTN.W;
  v4 RBN = projInv * _v4(frustumScale, -frustumScale, -frustumScale, 1.0f);
  RBN = RBN / RBN.W;
  v4 LBN = projInv * _v4(-frustumScale, -frustumScale, -frustumScale, 1.0f);
  LBN = LBN / LBN.W;
  v4 RTF = projInv * _v4(frustumScale, frustumScale, frustumScale, 1.0f);
  RTF = RTF / RTF.W;
  v4 LTF = projInv * _v4(-frustumScale, frustumScale, frustumScale, 1.0f);
  LTF = LTF / LTF.W;
  v4 RBF = projInv * _v4(frustumScale, -frustumScale, frustumScale, 1.0f);
  RBF = RBF / RBF.W;
  v4 LBF = projInv * _v4(-frustumScale, -frustumScale, frustumScale, 1.0f);
  LBF = LBF / LBF.W;

  glDisable(GL_DEPTH_TEST);
  DEBUG_RenderLine({ _v3(RTN), _v3(LBN - RTN) }, context, COLOR_RED);
  DEBUG_RenderLine({ _v3(RBN), _v3(LTN - RBN) }, context, COLOR_RED);

  DEBUG_RenderLine({ _v3(RTF), _v3(RTN - RTF) }, context, color);
  DEBUG_RenderLine({ _v3(LTF), _v3(LTN - LTF) }, context, color);
  DEBUG_RenderLine({ _v3(RBF), _v3(RBN - RBF) }, context, color);
  DEBUG_RenderLine({ _v3(LBF), _v3(LBN - LBF) }, context, color);

  DEBUG_RenderLine({ _v3(RTF), _v3(LTF - RTF) }, context, color);
  DEBUG_RenderLine({ _v3(LTF), _v3(LBF - LTF) }, context, color);
  DEBUG_RenderLine({ _v3(LBF), _v3(RBF - LBF) }, context, color);
  DEBUG_RenderLine({ _v3(RBF), _v3(RTF - RBF) }, context, color);
  glEnable(GL_DEPTH_TEST);
}

static void DEBUG_RenderCube(const v3& position, render_context& context)
{
  int cubeId = BindCube(context);

#ifndef GL_ES_VERSION_3_0
  GLint previousPolygonMode;
  glGetIntegerv(GL_POLYGON_MODE, &previousPolygonMode);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

  mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;
  SetUniform(context.SimpleShader.ViewProjection, &viewProjection, 1);

  mat4x4 modelMatrix = IDENTITY_MATRIX;

  // Actual position (where the body is)
  modelMatrix = mat4x4_translate(position) * mat4x4_scale(_v3(0.1f));
  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);

  glBindVertexArray(cubeId);
  glDrawArrays(GL_TRIANGLES, 0, 36);

#ifndef GL_ES_VERSION_3_0
  glPolygonMode(GL_FRONT_AND_BACK, previousPolygonMode);
#endif
}