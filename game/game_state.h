#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "game_assets.h"
#include "game_input.h"
#include "map_system.h"
#include "anim_system.h"
#include "ui_system.h"
#include "physics_system.h"
#include "ai_system.h"

using AnimationSystem::animation_run;
using AnimationSystem::animator;
using MapSystem::map;
using PhysicsSystem::space;

const size_t MAX_STATE_COMMANDS = 2048;
const size_t MAX_STATE_HOOKS = 128;

enum class game_phase
{
  SETUP,
  READY,
  STARTED,
  ENDING,
  ENDED,
  REPLAYING,
  RESULT,
  NONE,
  PERF_TEST
#if HOKI_DEV
#endif
};

enum game_state_command_type
{
  COMMAND_NOCOMMAND,
  COMMAND_RESTART_MAP,
  COMMAND_RESET_GAME,
  COMMAND_SHOW_GAME_RESULT,
  COMMAND_LOAD_ASSET,
  COMMAND_MOVE_CAMERA,

  COMMAND_PRIMARY_INPUT_START,
  COMMAND_PRIMARY_INPUT,
  COMMAND_PRIMARY_INPUT_END,

  COMMAND_OFFENSE_PREPARE_TO_SKATE,
  COMMAND_OFFENSE_START_SKATING,
  COMMAND_OFFENSE_SET_VELOCITY,
  COMMAND_OFFENSE_SHOOT,

  COMMAND_UPDATE_AIM_POSITION,
  COMMAND_PUCK_SET_POSITION,
  COMMAND_PUCK_SET_VELOCITY,

  COMMAND_ACTIVATE_JOYSTICK,

  COMMAND_SWITCH_PHASE,

  COMMAND_REPLAY_NO_MORE_DATA,

  COMMAND_SET_WIREFRAME,
  COMMAND_BUILD_SHADERS,
  COMMAND_CYCLE_SHADERS,

  COMMAND_GOALIE_MOVE,
  COMMAND_GOALIE_REACT,

#if HOKI_DEV
  DEBUG_COMMAND_TOGGLE_DEBUG_CAMERA,
  DEBUG_COMMAND_TOGGLE_MOUSEPICKER,
  DEBUG_COMMAND_TOGGLE_DEBUG_UI
#endif
};

struct game_state;
struct game_state_command;

#define STATE_ACTION(name) void name(game_state& state, game_state_command& cmd)
typedef STATE_ACTION(state_action_func);

struct game_state_hook
{
  game_state_command_type Type;
  state_action_func* Action;
  game_phase Phase;
};

struct game_state_hooks
{
  game_state_hook Hooks[MAX_STATE_HOOKS];
  size_t Count;
};

struct game_state_command
{
  double Delay;
  game_state_command_type Type;
  union
  {
    game_input_state InputState;
    v2 Coords;
    v3 Position;
  };
  bool Processed;
  bool __Pad[7];
};

struct game_state_command_buffer
{
  uint32_t Count;
  game_state_command Commands[MAX_STATE_COMMANDS];
};

struct game_state
{
  Asset::game_assets* Assets;

  int ToneVolume;
  int Hz;
  float TSine;

  float SimDelta;
  float FrameDelta;
  double SimTime;
  double RealTime;
  float TimeScale;
  float TickAccumulator;

  double ReadyTime;
  float PhaseDelay;
  game_phase NextPhase;
  game_phase Phase;

  double TimeAtLaunch;
  double GoalTime;
  v2 AimPosition;
  // Launcher 3
  float AimPower;
  float AimPowerDelta;
  float AimSpeed;

  // Launcher 2
  v2 AimPositionBuffer[5];
  int AimPositionBufferIndex;

  // Launcher 4
  v3 AimDir;

  game_state_hooks InputHooks;
  game_state_hooks StateHooks;
  game_state_hooks ReplayHooks;
  game_state_command_buffer StateCommands;
  game_state_command_buffer ReplayCommands;

  map Map;
  space PhysicsSpace;

  animator Animator;

  animation_run* Skate;
  animation_run* SkateHard;
  animation_run* ChargeUp;
  animation_run* Cheer;

  AISystem::ai_goalie AIGoalie;

  UISystem::ui_context UIContext;

  v3 ReplayPuckPos;
  v3 ReplayPuckNextPos;
  double ReplayPuckTime;
  double ReplayPuckStartTime;
  double ReplayPuckDelta;

  int GamesPlayed;
  int Goals;
  bool Won;

  float perfTestFpsLowQuality[5 * 60];
  int perfTestIndex;
  int perfTestPhase;
  float perfTestFpsHighQuality[5 * 60];

  bool SoundEnabled;
#if HOKI_DEV
  bool ShowDebugUI;
  bool MousePickerActive;
  game_entity* Picked;
  debug_render_line DebugAimLine;
  bool DebugCameraActive;
  game_camera DebugCamera;
  game_state_hooks DebugHooks;

#endif // HOKI_DEV
};

void push_state_command(game_state_command_buffer& buffer,
                        game_state_command_type type,
                        double delay = 0.0)
{
  HOKI_ASSERT(buffer.Count < ARRAY_SIZE(buffer.Commands));
  game_state_command& newCommand = buffer.Commands[buffer.Count++];
  newCommand = {};
  newCommand.Processed = false;
  newCommand.Type = type;
  newCommand.Delay = delay;
}

void push_state_command(game_state_command_buffer& buffer,
                        game_state_command_type type,
                        const game_input_state state)
{
  HOKI_ASSERT(buffer.Count < ARRAY_SIZE(buffer.Commands));
  game_state_command& newCommand = buffer.Commands[buffer.Count++];
  newCommand = {};
  newCommand.Type = type;
  newCommand.InputState = state;
}

void push_state_command(game_state_command_buffer& buffer,
                        game_state_command_type type,
                        const v2 coords)
{
  HOKI_ASSERT(buffer.Count < ARRAY_SIZE(buffer.Commands));
  game_state_command& command = buffer.Commands[buffer.Count++];
  command = {};
  command.Type = type;
  command.Coords = coords;
}

void push_state_command(game_state_command_buffer& buffer,
                        game_state_command_type type,
                        const v3 position,
                        double delay = 0.0f)
{
  HOKI_ASSERT(buffer.Count < ARRAY_SIZE(buffer.Commands));
  game_state_command& command = buffer.Commands[buffer.Count++];
  command = {};
  command.Type = type;
  command.Position = position;
  command.Delay = delay;
}

static void hook_action(game_state_hooks& buffer,
                        const game_phase phase,
                        const game_state_command_type type,
                        state_action_func action);

STATE_ACTION(store_command_to_replay_buffer);

#endif // GAME_STATE_H
