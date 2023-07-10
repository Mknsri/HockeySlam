#if HOKI_DEV
static void DEBUG_RenderPhysicsCollisions(const PhysicsSystem::body& body,
                                          render_context& context)
{
  int lineId = BindLine(context);
  glBindVertexArray(lineId);

  mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;
  SetUniform(context.SimpleShader.ViewProjection, &viewProjection, 1);

  mat4x4 modelMatrix = IDENTITY_MATRIX;
  for (size_t i = 0; i < body.CollisionCount; i++) {
    const PhysicsSystem::collision& collision = body.Collisions[i];
    const PhysicsSystem::body& other = *collision.Other;

    // Collision point
    v3 collColor = COLOR_RED * ((float)(i + 1) / (float)body.CollisionCount);
    modelMatrix =
      mat4x4_translate(collision.Point) * mat4x4_scale(collision.Normal * 0.5f);
    SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
    SetUniform(context.SimpleShader.ObjectColor, &collColor, 1);
    glDrawArrays(GL_LINES, 0, 2);

    // Collision normal
    v3 normalColor =
      COLOR_GREEN * ((float)(i + 1) / (float)body.CollisionCount);
    v3 offset =
      collision.Normal * abs_f(dot(collision.Normal, other.Size * 0.5f));
    modelMatrix = mat4x4_translate(other.State.Position + offset) *
                  mat4x4_scale(collision.Normal);
    SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
    SetUniform(context.SimpleShader.ObjectColor, &normalColor, 1);
    glDrawArrays(GL_LINES, 0, 2);

    // Collision penetration
    v3 penetColor = COLOR_BLUE * ((float)(i + 1) / (float)body.CollisionCount);
    modelMatrix =
      mat4x4_translate(collision.Point) * mat4x4_scale(-collision.Penetration);
    SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
    SetUniform(context.SimpleShader.ObjectColor, &penetColor, 1);
    glDrawArrays(GL_LINES, 0, 2);
  }
}

static void DEBUG_RenderPhysicsAABB(const PhysicsSystem::body& body,
                                    render_context& context)
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

  // Previous position (where the body was)
  modelMatrix =
    mat4x4_translate(body.PreviousState.Position) * mat4x4_scale(body.Size);
  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.SimpleShader.ObjectColor, &COLOR_WHITE, 1);

  glBindVertexArray(cubeId);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  //  Interpolated position (where the body will appear to be)
  modelMatrix =
    mat4x4_translate(body.InterpolatedPosition) * mat4x4_scale(body.Size);
  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.SimpleShader.ObjectColor, &COLOR_BLUE, 1);

  glBindVertexArray(cubeId);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  // Actual position (where the body is)
  modelMatrix = mat4x4_translate(body.State.Position) * mat4x4_scale(body.Size);
  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);

  if (body.TriggeredCount > 0) {
    SetUniform(context.SimpleShader.ObjectColor, &COLOR_YELLOW, 1);
  } else {
    SetUniform(context.SimpleShader.ObjectColor, &COLOR_CYAN, 1);
  }

  glBindVertexArray(cubeId);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  DEBUG_RenderPhysicsCollisions(body, context);
#ifndef GL_ES_VERSION_3_0
  glPolygonMode(GL_FRONT_AND_BACK, previousPolygonMode);
#endif
}

static void DEBUG_RenderPhysicsRay(const PhysicsSystem::body& body,
                                   render_context& context)
{
  int lineId = BindLine(context);

#ifndef GL_ES_VERSION_3_0
  GLint previousPolygonMode;
  glGetIntegerv(GL_POLYGON_MODE, &previousPolygonMode);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
  mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;
  SetUniform(context.SimpleShader.ViewProjection, &viewProjection, 1);

  mat4x4 modelMatrix = IDENTITY_MATRIX;

  // Previous position (where the body was)
  modelMatrix = mat4x4_translate(body.PreviousState.Position) *
                mat4x4_scale(body.PreviousState.Velocity);
  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.SimpleShader.ObjectColor, &COLOR_WHITE, 1);
  glBindVertexArray(lineId);
  glDrawArrays(GL_LINES, 0, 2);

  //  Interpolated position (where the body will appear to be)
  modelMatrix = mat4x4_translate(body.InterpolatedPosition) *
                mat4x4_scale(body.State.Velocity);
  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.SimpleShader.ObjectColor, &COLOR_BLUE, 1);
  glDrawArrays(GL_LINES, 0, 2);

  // Actual position (where the body is)
  modelMatrix =
    mat4x4_translate(body.State.Position) * mat4x4_scale(body.State.Velocity);
  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.SimpleShader.ObjectColor, &COLOR_BLACK, 1);
  glDrawArrays(GL_LINES, 0, 2);

  DEBUG_RenderPhysicsCollisions(body, context);

#ifndef GL_ES_VERSION_3_0
  glPolygonMode(GL_FRONT_AND_BACK, previousPolygonMode);
#endif
}
#endif