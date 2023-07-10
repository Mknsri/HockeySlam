#ifndef AI_SYSTEM_H
#define AI_SYSTEM_H

namespace AISystem {
enum class ai_goalie_state
{
  STATE_NONE = 0,
  STATE_IDLE,
  STATE_SLIDE_LEFT,
  STATE_SLIDE_RIGHT,
  STATE_TRACKING,
  STATE_LUNGE
};

enum class ai_goalie_lunge_dir
{
  LUNGE_NONE = 0,
  LUNGE_LEFT_HIGH,
  LUNGE_LEFT_MIDDLE,
  LUNGE_LEFT_LOW,

  LUNGE_MIDDLE_HIGH,
  LUNGE_MIDDLE_MIDDLE,
  LUNGE_MIDDLE_LOW,

  LUNGE_RIGHT_HIGH,
  LUNGE_RIGHT_MIDDLE,
  LUNGE_RIGHT_LOW,

};

struct ai_goalie
{
  ai_goalie_state State;
  ai_goalie_state NextState;
  ai_goalie_lunge_dir LungeDir;
  v3 Position;
  v3 Target;
  float MovementAmount;
  float Angle;
};
}

#endif // AI_SYSTEM_H