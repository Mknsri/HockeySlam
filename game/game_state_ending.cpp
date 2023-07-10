#include "game_state.h"

STATE_ACTION(puck_set_shoot_position)
{
  game_entity& puck = state.Map.Entities.Puck;
  PhysicsSystem::set_position(puck.Body, cmd.Position);
}

STATE_ACTION(puck_set_shoot_velocity)
{
  game_entity& puck = state.Map.Entities.Puck;
  PhysicsSystem::set_velocity(puck.Body, cmd.Position);
}

STATE_ACTION(offense_shoot)
{
  state.Skate = nullptr;
  state.SkateHard = nullptr;

  game_entity& puck = state.Map.Entities.Puck;
  game_entity& offense = state.Map.Entities.Offense;

  double shootAnimDelay = (state.SimTime - state.ReadyTime) +
                          offense_shoot_animation(state, cmd.Coords.X);

  v3 shootVel = get_shoot_velocity(state);
  v3 shootPos = offense_puck_shoot_offset(state);

  push_state_command(
    state.StateCommands, COMMAND_PUCK_SET_POSITION, shootPos, shootAnimDelay);
  push_state_command(
    state.StateCommands, COMMAND_PUCK_SET_VELOCITY, shootVel, shootAnimDelay);

  v3 goalPlanePoint = get_goal_plane_point(state);
  goalPlanePoint.Z = state.AIGoalie.Position.Z;
  v3 displacement = puck.Position - goalPlanePoint;
  state.AIGoalie.Target = goalPlanePoint;
  double reactionTime = 0.25f;
  double delay = (double)kinematic_get_time(
    displacement, shootVel, state.PhysicsSpace.Gravity);
  delay -= 0.29f; // Animation "hit time" is around 29ms

  // We're not gonna hit the goalie, dont react
  if (abs_f(goalPlanePoint.X) > 3.0f) {
    return;
  }

  v3 aiCenter = state.AIGoalie.Position + _v3(0.0f, 1.0f, 0.0f);
  v3 reactDirection = goalPlanePoint - aiCenter;

  delay += (state.SimTime - state.ReadyTime);
  push_state_command(
    state.StateCommands, COMMAND_GOALIE_REACT, reactDirection, delay);
}

static void setup_ending_phase(game_state& state)
{
  game_phase phase = game_phase::ENDING;
  hook_action(state.StateHooks, phase, COMMAND_OFFENSE_SHOOT, offense_shoot);
  hook_action(state.StateHooks,
              phase,
              COMMAND_OFFENSE_SHOOT,
              store_command_to_replay_buffer);
  hook_action(state.StateHooks,
              phase,
              COMMAND_PUCK_SET_VELOCITY,
              puck_set_shoot_velocity);
  hook_action(state.StateHooks,
              phase,
              COMMAND_PUCK_SET_VELOCITY,
              store_command_to_replay_buffer);
  hook_action(state.StateHooks,
              phase,
              COMMAND_PUCK_SET_POSITION,
              puck_set_shoot_position);
  hook_action(state.StateHooks,
              phase,
              COMMAND_PUCK_SET_POSITION,
              store_command_to_replay_buffer);
  hook_action(state.StateHooks, phase, COMMAND_GOALIE_REACT, goalie_react);
  hook_action(state.StateHooks, phase, COMMAND_GOALIE_MOVE, goalie_update_pos);
  hook_action(state.StateHooks,
              phase,
              COMMAND_GOALIE_REACT,
              store_command_to_replay_buffer);
  hook_action(state.StateHooks,
              phase,
              COMMAND_GOALIE_MOVE,
              store_command_to_replay_buffer);
}

static void tick_ending_phase(game_state& state,
                              game_memory& gameMemory,
                              render_context& renderContext)
{
#if HOKI_DEV
  add_debug_rendercommand(renderContext, &state.DebugAimLine);
#endif
  game_entity& puck = state.Map.Entities.Puck;
  game_entity& goal = state.Map.Entities.Posts;

  bool ended = state.NextPhase == game_phase::ENDED;
  bool shouldEnd = false;
  bool timeout = (state.SimTime > state.TimeAtLaunch + 2.0f);
  bool puckStopped = length(puck.Body->State.Velocity) < 1.0f;
  bool goalMade = goal.Body->TriggeredCount > 0;

  if (!ended && (puckStopped || timeout || goalMade)) {
    shouldEnd = true;
  }

  if (shouldEnd) {
    state.GamesPlayed++;
    state.Goals += goalMade ? 1 : 0;
    state.Cheer->Speed = goalMade ? 5.0f : 1.0f;
    state.PhaseDelay = goalMade ? 0.0f : 0.5f;
    state.NextPhase = game_phase::ENDED;
    state.GoalTime =
      goalMade ? state.SimTime - state.ReadyTime + state.PhaseDelay : 0.0f;
    if (goalMade) {
      PhysicsSystem::set_velocity(puck.Body, _v3(0.0f));
    }
  }

  tick_goalie(state);
  v3 nextPos =
    AISystem::get_goalie_next_pos(state.AIGoalie, state.AIGoalie.Target);
  push_state_command(state.StateCommands, COMMAND_GOALIE_MOVE, nextPos, 0.0);

  static v3 pos = _v3(0.0f);
  if (pos != puck.Body->State.Position) {
    pos = puck.Body->State.Position;
    push_state_command(state.ReplayCommands,
                       COMMAND_PUCK_SET_POSITION,
                       puck.Body->State.Position,
                       state.SimTime - state.ReadyTime);
    push_state_command(state.ReplayCommands,
                       COMMAND_PUCK_SET_VELOCITY,
                       puck.Body->State.Velocity,
                       state.SimTime - state.ReadyTime);
  }

#if HOKI_DEV
  add_debug_rendercommand(renderContext, &state.DebugAimLine);
#endif
}