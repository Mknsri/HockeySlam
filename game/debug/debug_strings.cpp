#ifndef DEBUG_STRINGS_H
#define DEBUG_STRINGS_H

#include "../asset/asset_shader.h"
#include "../game_state.h"

const char* debug_to_string(const game_phase phase)
{
  switch (phase) {
    case game_phase::SETUP:
      return "setup";
    case game_phase::READY:
      return "ready";
    case game_phase::STARTED:
      return "started";
    case game_phase::ENDING:
      return "ending";
    case game_phase::ENDED:
      return "ended";
    case game_phase::REPLAYING:
      return "replaying";
    case game_phase::PERF_TEST:
      return "perf test";

    default:
      return "unknown";
  }
}

const char* debug_to_string(const Asset::ogl_shader_type type)
{
  switch (type) {
    case Asset::ogl_shader_type::SHADER_TYPE_ANIMATED_MESH:
      return "ANIMATED_MESH";
    case Asset::ogl_shader_type::SHADER_TYPE_WEIGHTS:
      return "WEIGHTS";
    case Asset::ogl_shader_type::SHADER_TYPE_SIMPLE:
      return "SIMPLE";
    case Asset::ogl_shader_type::SHADER_TYPE_SIMPLE_TEXTURED:
      return "SIMPLE_TEXTURED";
    case Asset::ogl_shader_type::SHADER_TYPE_TEXT:
      return "TEXT";
    case Asset::ogl_shader_type::SHADER_TYPE_SHADOW:
      return "SHADOW";
    case Asset::ogl_shader_type::SHADER_TYPE_FILLED:
      return "FILLED";
    case Asset::ogl_shader_type::SHADER_TYPE_PBR:
      return "PBR";
    case Asset::ogl_shader_type::SHADER_TYPE_PBR_INSTANCED:
      return "SHADER_TYPE_PBR_INSTANCED";
    case Asset::ogl_shader_type::SHADER_TYPE_UI:
      return "UI";

    default:
      HOKI_ASSERT(false);
      return "UNKNOWN";
  }
}

const char* debug_to_string(const AISystem::ai_goalie_state state)
{
  switch (state) {
    case AISystem::ai_goalie_state::STATE_IDLE:
      return "IDLE";
    case AISystem::ai_goalie_state::STATE_LUNGE:
      return "LUNGE";
    case AISystem::ai_goalie_state::STATE_NONE:
      return "NONE";
    case AISystem::ai_goalie_state::STATE_SLIDE_LEFT:
      return "SLIDE_LEFT";
    case AISystem::ai_goalie_state::STATE_SLIDE_RIGHT:
      return "SLIDE_RIGHT";

    default:
      HOKI_ASSERT(false);
      return "UNKNOWN";
  }
}

const char* debug_to_string(const AISystem::ai_goalie_lunge_dir dir)
{
  switch (dir) {
    case AISystem::ai_goalie_lunge_dir::LUNGE_LEFT_HIGH:
      return "LUNGE_LEFT_HIGH";
    case AISystem::ai_goalie_lunge_dir::LUNGE_LEFT_MIDDLE:
      return "LUNGE_LEFT_MIDDLE";
    case AISystem::ai_goalie_lunge_dir::LUNGE_LEFT_LOW:
      return "LUNGE_LEFT_LOW";
    case AISystem::ai_goalie_lunge_dir::LUNGE_RIGHT_HIGH:
      return "LUNGE_RIGHT_HIGH";
    case AISystem::ai_goalie_lunge_dir::LUNGE_RIGHT_MIDDLE:
      return "LUNGE_RIGHT_MIDDLE";
    case AISystem::ai_goalie_lunge_dir::LUNGE_RIGHT_LOW:
      return "LUNGE_RIGHT_LOW";

    default:
      return "UNKNOWN";
  }
}

const char* debug_to_string(const game_state_command_type type)
{
  switch (type) {
    case COMMAND_NOCOMMAND:
      return "COMMAND_NOCOMMAND";
    case COMMAND_RESTART_MAP:
      return "COMMAND_RESTART_MAP";
    case COMMAND_LOAD_ASSET:
      return "COMMAND_LOAD_ASSET";
    case COMMAND_MOVE_CAMERA:
      return "COMMAND_MOVE_CAMERA";

    case COMMAND_PRIMARY_INPUT_START:
      return "COMMAND_PRIMARY_INPUT_START";
    case COMMAND_PRIMARY_INPUT:
      return "COMMAND_PRIMARY_INPUT";
    case COMMAND_PRIMARY_INPUT_END:
      return "COMMAND_PRIMARY_INPUT_END";

    case COMMAND_ACTIVATE_JOYSTICK:
      return "COMMAND_ACTIVATE_JOYSTICK";

    case COMMAND_SWITCH_PHASE:
      return "COMMAND_SWITCH_PHASE";

    case COMMAND_SET_WIREFRAME:
      return "COMMAND_SET_WIREFRAME";
    case COMMAND_BUILD_SHADERS:
      return "COMMAND_BUILD_SHADERS";
    case COMMAND_CYCLE_SHADERS:
      return "COMMAND_CYCLE_SHADERS";
#if HOKI_DEV
    case DEBUG_COMMAND_TOGGLE_DEBUG_CAMERA:
      return "DEBUG_COMMAND_TOGGLE_DEBUG_CAMERA";
    case DEBUG_COMMAND_TOGGLE_MOUSEPICKER:
      return "DEBUG_COMMAND_TOGGLE_MOUSEPICKER";
#endif
    case COMMAND_PUCK_SET_POSITION:
      return "COMMAND_PUCK_SET_POSITION";
    case COMMAND_PUCK_SET_VELOCITY:
      return "COMMAND_PUCK_SET_VELOCITY";
    case COMMAND_UPDATE_AIM_POSITION:
      return "COMMAND_UPDATE_AIM_POSITION";
    case COMMAND_GOALIE_MOVE:
      return "COMMAND_GOALIE_MOVE";
    case COMMAND_RESET_GAME:
      return "COMMAND_RESET_GAME";

    case COMMAND_OFFENSE_START_SKATING:
      return "COMMAND_OFFENSE_START_SKATING";
    case COMMAND_OFFENSE_SET_VELOCITY:
      return "COMMAND_OFFENSE_SET_VELOCITY";

    case COMMAND_OFFENSE_PREPARE_TO_SKATE:
      return "COMMAND_OFFENSE_PREPARE_TO_SKATE";

    default:
      HOKI_ASSERT_MESSAGE(false, "missing enum");
      return "UNKNOWN";
  }
}

#endif