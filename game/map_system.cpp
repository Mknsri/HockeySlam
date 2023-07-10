#include "map_system.h"
#include "game_camera.h"
#include "game_state.h"
#include "game_entity.h"

namespace MapSystem {

void load_map(game_state& state, map* outMap)
{
  // game_entity* stands = &outMap->Entities.Stands;
  // stands->Position = _v3(0.0f, 0.0f, 15.0f);

  game_entity* field = &outMap->Entities.Field;

  field->Position = _v3(0.0f, 0.0f, 15.0f);
  field->Scale = _v3(1.0);

  v3 fieldBodySize = _v3(35.0f, 1.0f, 90.0f);
  v3 fieldOffset = _v3(0.0f, fieldBodySize.Y * -0.55f, 0.0f);

  body& fieldBody = PhysicsSystem::add_static_body_to_entity(
    state.PhysicsSpace, *field, fieldOffset, fieldBodySize);
#if HOKI_DEV
  fieldBody.Name = "fieldBody";
#endif
  v3 backWallSize = _v3(35.0f, 3.8f, 0.25f);
  body& backWall = add_body_to_space(state.PhysicsSpace,
                                     _v3(0.0f, 2.0f, -25.0f),
                                     PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                     backWallSize);
  backWall.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  backWall.Name = "backWall";
#endif

  v3 sideWallSize = _v3(backWallSize.Z, backWallSize.Y, backWallSize.X * 2.0f);
  body& leftSide = add_body_to_space(state.PhysicsSpace,
                                     _v3(17.5f, 2.0f, 0.0f),
                                     PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                     sideWallSize);
  leftSide.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  leftSide.Name = "leftSide";
#endif
  body& rightSide = add_body_to_space(state.PhysicsSpace,
                                      _v3(-17.5f, 2.0f, 0.0f),
                                      PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                      sideWallSize);
  rightSide.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  rightSide.Name = "rightSide";
#endif

  /**
   * POSTS
   */
  game_entity* posts = &outMap->Entities.Posts;

  posts->Position = _v3(0.0f, 0.0f, -17.5f);
  posts->Rotation = quat_from_euler(_v3(0.0f, 0.0f, 00.0f));
  posts->Scale = _v3(1.2f);
  v3 netSize = _v3(5.2f, 3.0f, 0.25f);
  v3 netOffset = _v3(0.0f, netSize.Y * 0.5f, -2.0f);
  body& net = add_body_to_space(state.PhysicsSpace,
                                posts->Position + netOffset,
                                PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                netSize);
  net.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  net.Name = "net";
#endif
  body& netLower = add_body_to_space(state.PhysicsSpace,
                                     posts->Position + _v3(0.0f, 0.0f, -2.5f),
                                     PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                     _v3(netSize.X, 1.25f, -1.0f));
  netLower.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  netLower.Name = "netLower";
#endif
  v3 wallSize = _v3(0.25f, netSize.Y, 2.25f);
  v3 rWallOffset = _v3(netSize.X * 0.5f + wallSize.X * 0.5f,
                       wallSize.Y * 0.5f,
                       wallSize.Z * 0.5f + netOffset.Z);
  body& rightWall = add_body_to_space(state.PhysicsSpace,
                                      posts->Position + rWallOffset,
                                      PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                      wallSize);
  rightWall.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  rightWall.Name = "rightWall";
#endif

  v3 lWallOffset = _v3(-rWallOffset.X, rWallOffset.Y, rWallOffset.Z);
  body& leftWall = add_body_to_space(state.PhysicsSpace,
                                     posts->Position + lWallOffset,
                                     PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                     wallSize);
  leftWall.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  leftWall.Name = "leftWall";
#endif

  v3 roofSize = _v3(netSize.X, netSize.Z, wallSize.Z);
  v3 roofOffset = _v3(0.0f, netSize.Y, rWallOffset.Z);
  body& roof = add_body_to_space(state.PhysicsSpace,
                                 posts->Position + roofOffset,
                                 PhysicsSystem::PHYSICS_BODY_TYPE_AABB,
                                 roofSize);
  roof.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC;
#if HOKI_DEV
  roof.Name = "roof";
#endif

  v3 goalSize = _v3(netSize.X - wallSize.X, netSize.Y - roofSize.Y, 0.9f);
  v3 goalOffset = _v3(0.0f, goalSize.Y * 0.5f, netOffset.Z + wallSize.Z * 0.7f);
  body& goalBody = PhysicsSystem::add_body_to_entity(
    state.PhysicsSpace, *posts, goalOffset, goalSize);
  goalBody.Flags = PhysicsSystem::PHYSICS_BODY_FLAG_STATIC |
                   PhysicsSystem::PHYSICS_BODY_FLAG_TRIGGER;
#if HOKI_DEV
  goalBody.Name = "goalBody";
#endif

  /**
   * OFFENSE
   */
  game_entity* offense = &outMap->Entities.Offense;

  offense->Position = _v3(0.0f, 0.0f, 16.0f);
  offense->Rotation = quat_from_euler(_v3(0.0f, 180.0f, 0.0f));
  body& offenseBody = PhysicsSystem::add_body_to_entity(
    state.PhysicsSpace, *offense, _v3(0.0f, 0.51f, 0.0f), _v3(1.0f), 0.4f);
  offenseBody.Friction = 0.1f;
  offenseBody.Flags = PhysicsSystem::body_flags::PHYSICS_BODY_FLAG_TRIGGER;
#if HOKI_DEV
  offenseBody.Name = "offenseBody";
#endif

  /**
   * PUCK
   */
  game_entity* puck = &outMap->Entities.Puck;

  puck->Position = _v3(0.0f, 1.0f, offense->Position.Z - 1.0f);
  puck->Scale = _v3(0.5f);

  v3 puckOffset = _v3(0.0f, 0.025f, 0.0f);
  v3 puckBodySize = _v3(0.25f, 0.15f, 0.25f);
  body& puckBody = PhysicsSystem::add_body_to_entity(
    state.PhysicsSpace, *puck, puckOffset, puckBodySize);
  puckBody.Bounce = 0.2f;
#if HOKI_DEV
  puckBody.Name = "puckBody";
#endif

  /**
   * GOALIE
   */
  game_entity* goalie = &outMap->Entities.Goalie;
  
  goalie->Position = _v3(0.0f, 0.0f, -16.0f);
  goalie->Rotation = IDENTITY_ROTATION;
  add_bodies_to_bones(state.PhysicsSpace, *goalie);

  /**
   * SEATS
   */
  instanced_entity* seatEnd = &outMap->InstancedEntities.SeatEnd;
  seatEnd->Position = _v3(-12.0f, 12.0f, 58.0f);
  seatEnd->Scale = _v3(1.0f);
  seatEnd->Rotation = quat_from_euler(_v3(0.0f, 180.0f, 0.0f));
  seatEnd->InstanceSpacing = _v3(3.0f, -2.0f, 2.0f);
  seatEnd->InstanceCount = 55;
  instanced_entity* seatLSide = &outMap->InstancedEntities.SeatLSide;
  seatLSide->Position = _v3(-30.0f, 12.0f, 48.0f);
  seatLSide->Scale = _v3(1.0f);
  seatLSide->Rotation = quat_from_euler(_v3(0.0f, 90.0f, 0.0f));
  seatLSide->InstanceSpacing = _v3(3.0f, -2.0f, 2.0f);
  seatLSide->InstanceCount = 120;
  instanced_entity* seatRSide = &outMap->InstancedEntities.SeatRSide;
  seatRSide->Position = _v3(30.0f, 12.0f, -33.0f);
  seatRSide->Scale = _v3(1.0f);
  seatRSide->Rotation = quat_from_euler(_v3(0.0f, 270.0f, 0.0f));
  seatRSide->InstanceSpacing = _v3(3.0f, -2.0f, 2.0f);
  seatRSide->InstanceCount = 120;

  /**
   * CROWD
   */
  instanced_entity* crowdEnd = &outMap->InstancedEntities.CrowdEnd;
  crowdEnd->Position = _v3(-14.0f, 10.0f, 56.0f);
  crowdEnd->Scale = _v3(1.0f);
  crowdEnd->Rotation = quat_from_euler(_v3(0.0f, 180.0f, 0.0f));
  crowdEnd->InstanceSpacing = _v3(3.0f * 4.0f, -2.0f, 2.0f);
  crowdEnd->InstanceCount = 8;
  instanced_entity* crowdLSide = &outMap->InstancedEntities.CrowdLSide;
  crowdLSide->Position = _v3(-29.0f, 10.0f, 48.0f);
  crowdLSide->Scale = _v3(1.0f);
  crowdLSide->Rotation = quat_from_euler(_v3(0.0f, 90.0f, 0.0f));
  crowdLSide->InstanceSpacing = _v3(3.0f * 4.0f, -2.0f, 2.0f);
  crowdLSide->InstanceCount = 30;
  instanced_entity* crowdRSide = &outMap->InstancedEntities.CrowdRSide;
  crowdRSide->Position = _v3(29.0f, 10.0f, -33.0f);
  crowdRSide->Scale = _v3(1.0f);
  crowdRSide->Rotation = quat_from_euler(_v3(0.0f, 270.0f, 0.0f));
  crowdRSide->InstanceSpacing = _v3(3.0f * 4.0f, -2.0f, 2.0f);
  crowdRSide->InstanceCount = 30;

  /**
   * LIGHTS
   */
  float pointLightX = 8.0f;
  float pointLightY = 15.0f;
  float pointLightZ = 20.0f;
  float pointAmbient = 1.0f;
  float pointDiffuse = 0.6f;
  float pointSpecular = 0.9f;
  float dirAmbient = 0.08f;
  float dirDiffuse = 1.0f;  // Not in use
  float dirSpecular = 1.0f; // Not in use
  outMap->Lights[0] = create_directional_light(_v3(0.05f, -0.30f, -0.45f),
                                               COLOR_WHITE,
                                               dirAmbient,
                                               dirDiffuse,
                                               dirSpecular);
  outMap->Lights[1] = create_point_light(
    offense->Position + _v3(pointLightX, pointLightY, pointLightZ),
    30.0f,
    COLOR_WHITE * 0.1f,
    pointAmbient,
    pointDiffuse,
    pointSpecular);
  outMap->Lights[2] = create_point_light(
    offense->Position + _v3(-pointLightX, pointLightY, pointLightZ),
    30.0f,
    COLOR_WHITE,
    pointAmbient,
    pointDiffuse,
    pointSpecular);
  outMap->Lights[3] = create_point_light(
    offense->Position + _v3(pointLightX, pointLightY, -pointLightZ),
    30.0f,
    COLOR_WHITE,
    pointAmbient,
    pointDiffuse,
    pointSpecular);
  outMap->Lights[4] = create_point_light(
    offense->Position + _v3(-pointLightX, pointLightY, -pointLightZ),
    30.0f,
    COLOR_WHITE,
    pointAmbient,
    pointDiffuse,
    pointSpecular);

  v3 cameraTarget = _v3(offense->Position.X, 0.0f, offense->Position.Z - 5.0f);
  outMap->GameCamera =
    create_camera(_v3(0.0f, 3.5f, offense->Position.Z + 4.0f), cameraTarget);

  // Randomly pick one replay position
  game_camera replayCameraTop = create_camera(_v3(3.5f, 5.5f, -8.0f), puck);
  game_camera replayCameraAngle = create_camera(_v3(0.0f, 3.5f, -14.5f), puck);
  game_camera replayeCameraAbove =
    create_camera(posts->Position + _v3(0.0f, 6.0f, -1.0f), puck);
  game_camera cameras[] = { replayCameraTop,
                            replayCameraAngle,
                            replayeCameraAbove };

  outMap->ReplayCamera = cameras[rand() % ARRAY_SIZE(cameras)];
}

}
