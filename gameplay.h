#pragma once
#include "geo.h"
#include "misc.h"

#define CAVE_POS ((Vec2) { 6, 10 })

float fireball_flying_t(Fireball *f);
float fireball_charging_t(Fireball *f);
float fireball_dying_t(Fireball *f);
float fireball_t(Fireball *f);
Vec2 fireball_pos(Fireball *f);
void quest(Geo *geo, Mushroom **onscreen_mush, float dt);
void man_anim(Man *man, float dt, Vec2 vel);
Vec2 man_pos(Man *man, ManPartKind mpk);
void label_push_i(Label *l, int i, Vec2 p);
void fireballs_push(Vec2 start, Vec2 target);
typedef struct {
  double elapsed;

  /* graphics */
  Vert vbuf[1 << 16];
  uint16_t ibuf[1 << 17];
  int width, height;
  float zoom;
  Vec2 cam;
  ManFrames mf;

  /* networking */
  uint8_t netbuf[1 << 8];
  Other others[20];
  uint32_t id;
  float time_since_netout;

  /* input */
  struct {
    uint8_t active;
    struct {
      int x, y;
    } mouse_start;
    struct {
      float x, y;
    } cam_start;
  } drag;
  uint8_t keys_down[255];
  struct {
    Vec2 vel;
    Man man;
    PlayerAction action;

    Vec2 cast_target;
  } player;

  /* ui */
  float letter_width_buf[128];
  char todo[5][25];
  Label labels[64];

  /* gameplay? */
  Mushroom mushrooms[1 << 8];
  Fireball fireballs[1 << 6];

} State;

extern State state;
