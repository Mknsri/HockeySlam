#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include "game_assets.h"
#include "ui_system.h"
#include "game_entity.h"
#include "game_camera.h"
#include "game_light.h"
#include "physics_system.h"
#include "map_system.h"
#include "anim_system.h"

using Asset::decal;
using Asset::file;
using Asset::gradient;
using Asset::ogl_shader;
using Asset::ogl_shader_program;
using Asset::texture;
using MapSystem::map;
using PhysicsSystem::body;

struct game_window_info
{
  int Width;
  int Height;
};

const v3 COLOR_BLACK = _v3(0.0f);
const v3 COLOR_WHITE = _v3(1.0f);
const v3 COLOR_RED = _v3(1.0f, 0.0f, 0.0f);
const v3 COLOR_GREEN = _v3(0.0f, 1.0f, 0.0f);
const v3 COLOR_BLUE = _v3(0.0f, 0.0f, 1.0f);
const v3 COLOR_YELLOW = _v3(1.0f, 1.0f, 0.0f);
const v3 COLOR_PINK = _v3(1.0f, 0.0f, 1.0f);
const v3 COLOR_CYAN = _v3(0.0f, 1.0f, 1.0f);

enum render_command_type
{
  RENDER_COMMAND_TYPE_UNDEFINED,
  RENDER_COMMAND_TYPE_INITIALIZE,
  RENDER_COMMAND_TYPE_CREATE_SHADER,
  RENDER_COMMAND_TYPE_ENTITY,
  RENDER_COMMAND_TYPE_CAMERA,
  RENDER_COMMAND_TYPE_LIGHT,
  RENDER_COMMAND_TYPE_SHADOW,
  RENDER_COMMAND_TYPE_TEXT,
  RENDER_COMMAND_TYPE_OUTLINE,
  RENDER_COMMAND_TYPE_REFLECTION,
  RENDER_COMMAND_TYPE_TRANSLUCENT,
  RENDER_COMMAND_TYPE_INSTANCED,

  RENDER_COMMAND_TYPE_UI_CONTEXT,
  RENDER_COMMAND_TYPE_UI_BUTTON,
  RENDER_COMMAND_TYPE_UI_ICON,
  RENDER_COMMAND_TYPE_UI_TOGGLE,
  RENDER_COMMAND_TYPE_UI_JOYSTICK,
  RENDER_COMMAND_TYPE_UI_SLIDER,

  DEBUG_RENDER_COMMAND_TYPE_SET_WIREFRAME,
  DEBUG_RENDER_COMMAND_TYPE_CYCLE_SHADERS,
  DEBUG_RENDER_COMMAND_TYPE_BONES,
  DEBUG_RENDER_COMMAND_TYPE_PHYSICS_BODY,
  DEBUG_RENDER_COMMAND_TYPE_LINE,
  DEBUG_RENDER_COMMAND_TYPE_UI_LAYER,
  DEBUG_RENDER_COMMAND_TYPE_LIGHT,
  DEBUG_RENDER_COMMAND_TYPE_CUBE
};

struct debug_render_line
{
  v3 Position;
  v3 Length;
};

struct renderable
{
  uint32_t RenderId;
};

enum render_data_flags
{
  NO_FLAGS = 0x0,
  RENDER_TO_SHADOWMAP = 0x1,
  RENDER_TO_STENCIL_BUFFER = 0x2
};

struct render_command
{
  render_command_type Type;

  uint32_t Flags;
  union
  {
    const ogl_shader_program* ShaderProgram;
    const game_entity* Entity;
    const instanced_entity* InstancedEntity;
    const game_camera* Camera;
    const game_light* Light;
    const debug_render_line* Line;
    const v3* CubePos;
    const body* Body;
    const map* Map;
    const UISystem::ui_icon* Icon;
    const UISystem::ui_text* Text;
    const UISystem::ui_button* Button;
    const UISystem::ui_toggle* Toggle;
    const UISystem::ui_joystick* Joystick;
    const UISystem::ui_slider* Slider;
    UISystem::ui_context* UIContext;
  };
};

struct render_command_buffer
{
  render_command Commands[100];
  uint32_t Count;
};

#endif
