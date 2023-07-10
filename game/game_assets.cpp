#include "game_assets.h"

namespace Asset {

struct model_load_args
{
  model* Result;
  const char* Path;
  game_memory* Memory;
};

PLATFORM_WORK_QUEUE_CALLBACK(LoadModelAsync)
{
  model_load_args* args = (model_load_args*)data;
  *args->Result = Asset::loadModel(args->Path, *args->Memory);
}

static game_assets load_assets(game_memory& memory)
{
  game_assets assets = {};

#if HOKI_DEV
  ogl_shader* WeightedVertexShader =
    Asset::loadShader("/res/shaders/gl/vertexshader_weights.vert", memory);
  ogl_shader* WeightedFragmentShader =
    Asset::loadShader("/res/shaders/gl/fragmentshader_weights.frag", memory);
  assets.WeightedShaderProgram = create_shader_program(
    WeightedVertexShader, WeightedFragmentShader, Asset::SHADER_TYPE_WEIGHTS);
#endif
  ogl_shader* SimpleVertexShader =
    Asset::loadShader("/res/shaders/gl/simple.vert", memory);
  ogl_shader* SimpleFragmentShader =
    Asset::loadShader("/res/shaders/gl/simple.frag", memory);
  assets.SimpleShaderProgram = create_shader_program(
    SimpleVertexShader, SimpleFragmentShader, Asset::SHADER_TYPE_SIMPLE);

  ogl_shader* SimpleTexturedVertexShader =
    Asset::loadShader("/res/shaders/gl/simple_textured.vert", memory);
  ogl_shader* SimpleTexturedFragmentShader =
    Asset::loadShader("/res/shaders/gl/simple_textured.frag", memory);
  assets.SimpleTexturedProgram =
    create_shader_program(SimpleTexturedVertexShader,
                          SimpleTexturedFragmentShader,
                          Asset::SHADER_TYPE_SIMPLE_TEXTURED);

  ogl_shader* VertexShader =
    Asset::loadShader("/res/shaders/preprocessed/gl/vertexshader.vert", memory);
  ogl_shader* FragmentShader = Asset::loadShader(
    "/res/shaders/preprocessed/gl/fragmentshader.frag", memory);
  assets.ShaderProgram = create_shader_program(
    VertexShader, FragmentShader, Asset::SHADER_TYPE_ANIMATED_MESH);

  ogl_shader* PbrFragmentShader =
    Asset::loadShader("/res/shaders/preprocessed/gl/pbr.frag", memory);
  assets.PbrShaderProgram = create_shader_program(
    VertexShader, PbrFragmentShader, Asset::SHADER_TYPE_PBR);

  ogl_shader* PbrInstancedVertexShader = Asset::loadShader(
    "/res/shaders/preprocessed/gl/vertex_shader_inst.vert", memory);
  assets.PbrInstancedShaderProgram =
    create_shader_program(PbrInstancedVertexShader,
                          PbrFragmentShader,
                          Asset::SHADER_TYPE_PBR_INSTANCED);

  ogl_shader* TextVertexShader =
    Asset::loadShader("/res/shaders/gl/text.vert", memory);
  ogl_shader* TextFragmentShader =
    Asset::loadShader("/res/shaders/gl/text.frag", memory);
  assets.TextShaderProgram = create_shader_program(
    TextVertexShader, TextFragmentShader, Asset::SHADER_TYPE_TEXT);

  ogl_shader* SimpleSkinnedVertexShader =
    Asset::loadShader("/res/shaders/gl/simple_skinned.vert", memory);
  ogl_shader* NoOpFragmentShader =
    Asset::loadShader("/res/shaders/gl/noop.frag", memory);
  assets.ShadowShaderProgram = create_shader_program(
    SimpleSkinnedVertexShader, NoOpFragmentShader, Asset::SHADER_TYPE_SHADOW);

  ogl_shader* FilledFragmentShader =
    Asset::loadShader("/res/shaders/gl/filled.frag", memory);
  assets.FillRevealShader = create_shader_program(SimpleTexturedVertexShader,
                                                  FilledFragmentShader,
                                                  Asset::SHADER_TYPE_FILLED);

  ogl_shader* UIFragmentShader =
    Asset::loadShader("/res/shaders/gl/ui.frag", memory);
  assets.UIShaderProgram = create_shader_program(
    SimpleTexturedVertexShader, UIFragmentShader, Asset::SHADER_TYPE_UI);

  file fontFile = Asset::load_file("/res/fonts/verdana.ttf", memory);
  assets.TestFont = Asset::initializeFont(fontFile);

  assets.OffenseModel =
    Asset::loadModel("/res/models/hockeyplayer.glb", memory);
  assets.GoalieModel = Asset::loadModel("/res/models/goalie.glb", memory);
  assets.FieldModel = Asset::loadModel("/res/models/field.glb", memory);
  assets.StandsModel = Asset::loadModel("/res/models/stands.glb", memory);
  assets.PostsModel = Asset::loadModel("/res/models/posts.glb", memory);
  assets.SeatModel = Asset::loadModel("/res/models/seat.glb", memory);
  assets.CrowdModel = Asset::loadModel("/res/models/crowd.glb", memory);
  assets.ArrowModel = Asset::loadModel("/res/models/arrow.glb", memory);

  // assets.SneikModel = Asset::loadModel("/res/models/puck.gltf", memory);

  // assets.IceTexture = Asset::loadTexture("/res/textures/field_uv.png",
  // memory); assets.TestTexture =
  // Asset::loadTexture("/res/textures/hokiman_texture.jpg", memory);
  // assets.PostsTexture =
  // Asset::loadTexture("/res/textures/maalitolpat_tex.png", memory);
  assets.PuckModel = Asset::loadModel("/res/models/puck.glb", memory);
  assets.CursorTexture =
    Asset::loadTexture("/res/textures/pointer.png", memory);
  assets.DebugDirectionTexture =
    Asset::loadTexture("/res/textures/dir_pointer.png", memory);
  // assets.GoalieTexture = Asset::loadTexture("/res/textures/mualimies.png",
  // memory);

  assets.LogoTexture = Asset::loadTexture("/res/textures/logovec.png", memory);
  assets.ButtonTexture = Asset::loadTexture("/res/textures/brb.png", memory);
  assets.ToggleTexture = Asset::loadTexture("/res/textures/switch.png", memory);
  assets.SoundToggleTexture =
    Asset::loadTexture("/res/textures/sound_toggle.png", memory);

  assets.JoystickHandleTexture =
    Asset::loadTexture("/res/textures/jstick_handle.png", memory);
  assets.JoystickBgTexture =
    Asset::loadTexture("/res/textures/jstick_base.png", memory);
  assets.SliderHandleTexture =
    Asset::loadTexture("/res/textures/slider_handle.png", memory);
  assets.SliderBgTexture =
    Asset::loadTexture("/res/textures/slider_background.png", memory);

  assets.TutorialTexture =
    Asset::loadTexture("/res/textures/tutorial.png", memory);
  assets.GoalTextTexture =
    Asset::loadTexture("/res/textures/goal_text.png", memory);
  assets.GoalFlash1Texture =
    Asset::loadTexture("/res/textures/goalflash_1.png", memory);
  assets.PuckFlashTexture =
    Asset::loadTexture("/res/textures/puck_flash.png", memory);

  assets.GoalBackboardTexture =
    Asset::loadTexture("/res/textures/scoreboard.png", memory);
  assets.GoalCounterZeroGoal =
    Asset::loadTexture("/res/textures/goals_0.png", memory);
  assets.GoalCounterOneGoal =
    Asset::loadTexture("/res/textures/goals_1.png", memory);
  assets.GoalCounterTwoGoal =
    Asset::loadTexture("/res/textures/goals_2.png", memory);
  assets.GoalCounterThreeGoal =
    Asset::loadTexture("/res/textures/goals_3.png", memory);
  assets.GoalCounterResetButtonTexture =
    Asset::loadTexture("/res/textures/goalcounter_reset.png", memory);

  assets.WinEndTexture =
    Asset::loadTexture("/res/textures/you_win.png", memory);
  assets.NoWinEndTexture =
    Asset::loadTexture("/res/textures/no_win.png", memory);
  assets.EndResetTexture =
    Asset::loadTexture("/res/textures/restart.png", memory);

  return assets;
}

static void compile_shaders(render_context& context, game_assets& assets)
{
#if HOKI_DEV
  add_rendercommand(context, assets.WeightedShaderProgram);
#endif
  add_rendercommand(context, assets.SimpleShaderProgram);
  add_rendercommand(context, assets.SimpleTexturedProgram);
  add_rendercommand(context, assets.ShaderProgram);
  add_rendercommand(context, assets.TextShaderProgram);
  add_rendercommand(context, assets.ShadowShaderProgram);
  add_rendercommand(context, assets.FillRevealShader);
  add_rendercommand(context, assets.UIShaderProgram);
  add_rendercommand(context, assets.PbrShaderProgram);
  add_rendercommand(context, assets.PbrInstancedShaderProgram);
}
}