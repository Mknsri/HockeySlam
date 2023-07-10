#ifndef MAP_SYSTEM_H
#define MAP_SYSTEM_H

#include "game_light.h"
#include "game_entity.h"
#include "game_camera.h"

namespace MapSystem {
struct map_entity_list
{
  game_entity Field;
  game_entity Posts;
  game_entity Offense;
  game_entity Puck;
  game_entity Goalie;
  game_entity Arrow;
};
struct instanced_entity_list
{
  instanced_entity SeatLSide;
  instanced_entity SeatRSide;
  instanced_entity SeatEnd;
  instanced_entity CrowdLSide;
  instanced_entity CrowdRSide;
  instanced_entity CrowdEnd;
};

const size_t ENTITY_COUNT = sizeof(map_entity_list) / sizeof(game_entity);
const size_t INSTANCED_ENTITY_COUNT =
  sizeof(instanced_entity_list) / sizeof(instanced_entity);

struct map

{
  union
  {
    game_entity EntitiesList[ENTITY_COUNT];
    map_entity_list Entities;
  };
  union
  {
    instanced_entity InstancedEntitiesList[INSTANCED_ENTITY_COUNT];
    instanced_entity_list InstancedEntities;
  };

  game_light Lights[5];
  game_camera GameCamera;
  game_camera ReplayCamera;
};
}

#endif // MAP_SYSTEM_H