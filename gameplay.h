#pragma once
#include "misc.h"
#include "geo.h"

static float fireball_t(Fireball *f);
static Vec2 fireball_pos(Fireball *f);
static void quest(Geo *geo, Mushroom **onscreen_mush, float dt);
static void man_anim(Man *man, float dt, Vec2 vel);
static Vec2 man_pos(Man *man, ManPartKind mpk);
static void label_push_i(Label *l, int i, Vec2 p);