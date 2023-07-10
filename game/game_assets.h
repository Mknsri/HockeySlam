#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "asset/asset_shader.h"
#include "asset/asset_model.h"
#include "asset/asset_text.h"
#include "asset/asset_texture.h"
#include "asset/asset_gradient.h"
#include "asset/asset_decal.h"

namespace Asset {
struct game_assets
{
  ogl_shader_program ShaderProgram;
  ogl_shader_program WeightedShaderProgram;
  ogl_shader_program SimpleShaderProgram;
  ogl_shader_program UIShaderProgram;
  ogl_shader_program SimpleTexturedProgram;
  ogl_shader_program TextShaderProgram;
  ogl_shader_program ShadowShaderProgram;
  ogl_shader_program FillRevealShader;
  ogl_shader_program PbrShaderProgram;
  ogl_shader_program PbrInstancedShaderProgram;

  font TestFont;

  model OffenseModel;
  model FieldModel;
  model StandsModel;
  model SeatModel;
  model CrowdModel;
  model PostsModel;
  model SneikModel;
  model PuckModel;
  model GoalieModel;
  model ArrowModel;

  texture PostsTexture;
  texture TestTexture;
  texture GoalieTexture;

  texture LogoTexture;
  texture CursorTexture;
  texture ButtonTexture;
  texture ToggleTexture;
  texture SoundToggleTexture;
  texture JoystickHandleTexture;
  texture JoystickBgTexture;
  texture TutorialTexture;
  texture GoalTextTexture;
  texture GoalFlash1Texture;
  texture PuckFlashTexture;
  texture SliderBgTexture;
  texture SliderHandleTexture;

  texture GoalBackboardTexture;
  texture GoalCounterZeroGoal;
  texture GoalCounterOneGoal;
  texture GoalCounterTwoGoal;
  texture GoalCounterThreeGoal;
  texture GoalCounterResetButtonTexture;

  texture WinEndTexture;
  texture NoWinEndTexture;
  texture EndResetTexture;

  texture DebugDirectionTexture;
};
}

#endif // GAME_ASSETS_H