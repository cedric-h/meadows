#include <stdint.h>
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
/* an asset mostly generated by a python script */
#include "gameplay.h"
#include "geo.h"
#include "man.h"
#include "math.h"
#include "misc.h"
/* send out over network */
extern void netout(void *ptr, int len);

WASM_EXPORT Color color_picked = {0.0f, 0.0f, 0.0f, 1.0f};

/* note: doesn't un-zoom (should it?) */
static Vec2 px_to_world_space(int x, int y) {
  float aspect = (float)state.height / (float)state.width;
  Vec2 ret = {x, y};
  ret.x /= state.width;
  ret.y /= state.height;

  ret.x /= aspect;
  return ret;
}

static void cam_apply(Vert *start, Vert *end, float aspect) {
  float min_z = -1.0f;
  float max_z = 1.0f;

  for (Vert *p = start; p != end; p++)
    min_z = (p->z < min_z) ? p->z : min_z,
    max_z = (p->z > max_z) ? p->z : max_z;

  /* more width: show more
     more height: zoom in */
  for (Vert *p = start; p != end; p++) {
    p->pos.x += state.cam.x;
    p->pos.y += state.cam.y;

    p->pos.x *= aspect;
    p->z = ((p->z - min_z) / (max_z - min_z)) * 2.0f - 1.0f;
    p->z *= 0.99999f;

    // zoom
    p->pos.x /= state.zoom;
    p->pos.y /= state.zoom;

    // 0..1 is good actually
    p->pos.x = p->pos.x * 2.0f - 1.0f;
    p->pos.y = p->pos.y * 2.0f - 1.0f;
  }
}

WASM_EXPORT void netinit(void) {
  // float f = 42.001337f;
  // netout(&f, sizeof(f));
}

WASM_EXPORT void netin(int len) { /* netin ... yahoo? LMFAO */
  NetMsg *msg = (NetMsg *)state.netbuf;

  /* update existing Other if we already have this ID */
  for (int i = 0; i < ARR_LEN(state.others); i++) {
    Other *other = state.others + i;

    if (other->id == msg->id) {
      other->man = msg->man;
      return;
    }
  }

  /* otherwise, allocate new Other for this ID */
  for (int i = 0; i < ARR_LEN(state.others); i++) {
    Other *other = state.others + i;

    /* haha theres no way you randomly generated 0... right? */
    if (other->id)
      continue;

    other->id = msg->id;
    other->man = msg->man;
    return;
  }
}

/* you can always rewrite netcode to make an existing game secure.
 * .. what you can't do ...
 * is get back time you spent securing a game nobody will ever play. */

WASM_EXPORT float *letter_width_buf(void) { return state.letter_width_buf; }
WASM_EXPORT void teleport(float x, float y) {
  state.player.man.pos = (Vec2){x, y};
}
WASM_EXPORT void init(void) {
  netbuf(state.netbuf, ARR_LEN(state.netbuf));
  state.id = randf() * (float)(UINT32_MAX); // TODO: precision?

  state.player.man.pos = (Vec2){-1.0f, -0.5f};
  state.player.man.hp = state.player.man.max_hp = 1.0f;

  state.zoom = 5.0f;
  vbuf(state.vbuf, ARR_LEN(state.vbuf));
  ibuf(state.ibuf, ARR_LEN(state.ibuf));

  man_frames_fill(&state.mf);
  typedef struct {
    ManPartKind lhs, rhs;
  } PartPair;
  PartPair mirror[] = {
      {ManPartKind_Knee_L, ManPartKind_Knee_R},
      {ManPartKind_Sole_L, ManPartKind_Sole_R},
      {ManPartKind_Toe_L, ManPartKind_Toe_R},
      {ManPartKind_Elbow_L, ManPartKind_Elbow_R},
      {ManPartKind_Hand_L, ManPartKind_Hand_R},
  };
  ManPartKind no[] = {
      ManPartKind_Elbow_L,
      ManPartKind_Elbow_R,
      ManPartKind_Hand_L,
      ManPartKind_Hand_R,
  };

  state.mf.frames[5] = state.mf.frames[1];
  state.mf.frames[6] = state.mf.frames[2];
  state.mf.frames[7] = state.mf.frames[3];

  // for (int i = 0; i < ARR_LEN(no); i++) {
  //   ManPartKind mpk = no[i];
  //   state.mf.frames[5].pos[mpk] = state.mf.frames[1].pos[mpk];
  //   state.mf.frames[6].pos[mpk] = state.mf.frames[2].pos[mpk];
  //   state.mf.frames[7].pos[mpk] = state.mf.frames[3].pos[mpk];
  //   state.mf.frames[8].pos[mpk] = state.mf.frames[0].pos[mpk];
  // }

  /* pull arms in (less chonky stick man) */
  for (int j = 0; j < ManPartKind_COUNT; j++)
    for (int i = 0; i < ARR_LEN(no); i++) {
      ManPartKind n = no[i];
      state.mf.frames[j].pos[n].x *= 0.8f;
    }

  /* mirror animation data for other leg (only have half animation) */
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < ARR_LEN(mirror); i++) {
      ManPartKind lhs = mirror[i].lhs;
      ManPartKind rhs = mirror[i].rhs;
      state.mf.frames[5 + j].pos[lhs].y = state.mf.frames[1 + j].pos[rhs].y;
      state.mf.frames[5 + j].pos[lhs].z = state.mf.frames[1 + j].pos[rhs].z;
      state.mf.frames[5 + j].pos[rhs].y = state.mf.frames[1 + j].pos[lhs].y;
      state.mf.frames[5 + j].pos[rhs].z = state.mf.frames[1 + j].pos[lhs].z;
    }
  }

  /* pull arm anims toward neck on Y axis */
  for (int j = 0; j < ManPartKind_COUNT; j++) {
    for (int i = 0; i < ARR_LEN(no); i++) {
      ManPartKind n = no[i];
      float center = state.mf.frames[j].pos[ManPartKind_Neck].y;
      state.mf.frames[j].pos[n].y -= center;
      state.mf.frames[j].pos[n].y *= 0.35f;
      state.mf.frames[j].pos[n].y += center;
    }
  }
}

typedef enum {
  MouseEventKind_Down,
  MouseEventKind_Up,
  MouseEventKind_Move,
} MouseEventKind;
WASM_EXPORT void mouse(MouseEventKind mouse_event_kind, int x, int y) {
  switch (mouse_event_kind) {
  case MouseEventKind_Up:
    state.drag.active = 0;
    break;
  case MouseEventKind_Down:
    if (!state.drag.active) {
      state.drag.active = 1;
      state.drag.mouse_start.x = x;
      state.drag.mouse_start.y = y;
      state.drag.cam_start.x = state.cam.x;
      state.drag.cam_start.y = state.cam.y;
    }
    break;
  case MouseEventKind_Move:
    if (state.drag.active) {
      Vec2 p = px_to_world_space(x - state.drag.mouse_start.x,
                                 y - state.drag.mouse_start.y);
      state.cam.x = state.drag.cam_start.x + p.x * state.zoom;
      state.cam.y = state.drag.cam_start.y - p.y * state.zoom;
    }
    break;
  }
}

WASM_EXPORT void key(uint8_t down, char key) {
  state.keys_down[(int)key] = down;
}

WASM_EXPORT void zoom(int x, int y, float delta_pixels) {
  float t = 1.0f - delta_pixels / fmaxf(state.width, state.height);
#define X_SIZE (state.zoom * (float)state.width)
#define Y_SIZE (state.zoom * (float)state.height)
  float x_size_before = X_SIZE;
  float y_size_before = Y_SIZE;

  /* --- apply zoom --- */
  state.zoom *= t;

  Vec2 p = px_to_world_space((x_size_before - X_SIZE) / 2.0f,
                             (y_size_before - Y_SIZE) / 2.0f);
  state.cam.x -= p.x;
  state.cam.y -= p.y;

#undef X_SIZE
#undef Y_SIZE
}

WASM_EXPORT void frame(int width, int height, double _dt) {
  float dt = _dt / 1000.0;
  state.elapsed += dt;

  /* tell everyone where we are every 0.2 seconds */
  state.time_since_netout += dt;
  if (state.time_since_netout > 0.2f) {
    state.time_since_netout = 0.0f;

    NetMsg msg = {state.id, state.player.man};
    netout(&msg, sizeof(msg));
  }

  /* passing this down to geo fns is annoying */
  state.width = width;
  state.height = height;

  Geo geo = {
      .vbuf = state.vbuf,
      .vbuf_base = state.vbuf,
      .vbuf_max = ARR_LEN(state.vbuf),
      .ibuf = state.ibuf,
      .ibuf_base = state.ibuf,
      .ibuf_max = ARR_LEN(state.ibuf),
  };
  __builtin_memset(state.ibuf, 0, sizeof(state.ibuf));

  float aspect = (float)height / (float)width;

  // geo_rect(&geo, COLOR_GREEN, -0.1f, 0.1f,0.1f, 0.2f,0.2f);
  // geo_rect(&geo, COLOR_RED  , -0.2f, 0.0f,0.0f, 0.2f,0.2f);
  // geo_rect(&geo, COLOR_BLUE , -0.3f, 0.2f,0.2f, 0.2f,0.2f);

  Vec2 move = {};
  if (state.keys_down['w'])
    move.y += 1;
  if (state.keys_down['s'])
    move.y -= 1;
  if (state.keys_down['a'])
    move.x -= 1;
  if (state.keys_down['d'])
    move.x += 1;
  if (dot2(move, move) > 0.0f)
    move = norm2(move);

  /* friction */
  state.player.vel = mul2_f(state.player.vel, 0.85f);

  switch (state.player.action) {
  case PlayerAction_Walking: {

    state.player.vel = add2(state.player.vel, mul2_f(move, 0.002f));

    /* stops the player from "drifting" to a stop so much */
    if (dot2(state.player.vel, state.player.vel) > 0.00008f) {
      /* only adding to the pos when the anim is going is prolly weird
       * but it looks good so like ... */
      state.player.man.pos = add2(
          state.player.man.pos,
          mul2_f(norm2(state.player.vel), 0.013f * state.player.man.anim_damp));
    }

    man_anim(&state.player.man, dt, state.player.vel);

    if (state.keys_down[' ']) {
      state.player.action = PlayerAction_Casting;
      state.player.cast_target = (Vec2){0};
    }
  } break;
  case PlayerAction_Casting: {

    state.player.vel = add2(state.player.vel, mul2_f(move, 0.003f));

    state.player.cast_target = add2(state.player.cast_target, state.player.vel);

    man_anim(&state.player.man, dt, (Vec2){});

    Color magic = {0.2f, 0.64f, 0.8f, 1.0f};
    Vec2 p = add2(state.player.cast_target, state.player.man.pos);
    for (int u = 0; u < 5; u++) {
      float ur = state.elapsed * 3.0f + ((float)u / 5.0f) * (M_PI * 2.0f);
      Vec2 up = {p.x + cosf(ur) * 0.25f, p.y + sinf(ur) * 0.25f};

      geo_8gon(&geo, magic, up.y, up.x, up.y, 0.01f);

      for (int v = 0; v < 5; v++) {
        float vr = state.elapsed * 3.0f + ((float)v / 5.0f) * (M_PI * 2.0f);
        Vec2 vp = {p.x + cosf(vr) * 0.25f, p.y + sinf(vr) * 0.25f};

        Color m = magic;
        m.r -= 0.1f;
        m.g -= 0.1f;
        m.b -= 0.1f;
        geo_line(&geo, m, fmaxf(up.y, vp.y), up, vp, 0.02f);
      }
    }

    if (!state.keys_down[' ']) {
      state.player.action = PlayerAction_Walking;

      Vec2 start = add2(state.player.man.pos, (Vec2){0, 0.75f});
      Vec2 target = p;
      fireballs_push(start, target);
    }
  } break;
  }

  geo_man_id(&geo, &state.player.man, state.id);

  {
    static Man pacing_man = {.hp = 1.0f};
    float t = state.elapsed * 60.0f;

    Vec2 vel = {cosf(t * 0.01f + M_PI / 2), sinf(t * 0.01f + M_PI / 2)};
    Vec2 pos = {1.00f * cosf(t * 0.01f), 1.00f * sinf(t * 0.01f)};

    pacing_man.pos = pos;
    man_anim(&pacing_man, dt, vel);

    geo_man_id(&geo, &pacing_man, 0);
  }

  for (int i = 0; i < ARR_LEN(state.others); i++) {
    Other *other = state.others + i;
    if (other->id)
      geo_man_id(&geo, &other->man, other->id);
  }

  /* lerp cam towards player */
  if (!state.drag.active) {
    Vec2 halsc = px_to_world_space((state.zoom * (float)state.width) / 2.0f,
                                   (state.zoom * (float)state.height) / 2.0f);
    Vec2 head = man_pos(&state.player.man, ManPartKind_Head);
    Vec2 p = mul2_f(add2(state.player.man.pos, head), 0.5f);
    state.cam = (Vec2){lerp(state.cam.x, -p.x + halsc.x, dt * 2.4f),
                       lerp(state.cam.y, -p.y + halsc.y, dt * 2.4f)};
  }

  // geo_man(&geo, dt, 0.3f*cosf(dt*0.001f+M_PI/2),
  //                   0.3f*sinf(dt*0.001f+M_PI/2));

  float pad = 1.5f;
  Vec2 min = {-round_tof(state.cam.x, 0.1f) - pad,
              -round_tof(state.cam.y, 0.1f) - pad - 2.0f};
  Vec2 max = {min.x + state.zoom / aspect + pad * 2,
              min.y + state.zoom + pad * 2 + 1.0f};

  Mushroom *onscreen_mush[1 << 8] = {0};
  int onscreen_mush_i = 0;

  for (float _x = min.x; _x < max.x; _x += 0.1f)
    for (float _y = min.y; _y < max.y; _y += 0.1f) {
      /* the 0.000001f bias prevents -1.0f from getting trunced to 0 */
      int ix = (_x + 0.0001f * signf(_x)) / 0.1f;
      int iy = (_y + 0.0001f * signf(_y)) / 0.1f;

      /* the 0.05f bias centers things in their tiles */
      float x = _x + 0.05f;
      float y = _y + 0.05f;

      float gpn =
          fabsf(stb_perlin_fbm_noise3(x * 0.4f, y * 0.4f, 0.0f, 2, 0.5f, 6)) *
          3;
      gpn = fminf(gpn, 2.0f);
      // geo_rect(&geo, COLOR_GRASS_TOP, y, x,y, size,size);
      Color c = COLOR_GRASS_TOP;
      float gx = realmod(iy, 2) ? x + 0.05f : x;
      float gy = y;
      geo_tri(&geo, (Vert){.color = c, .z = y, .pos = {gx - 0.01f * gpn, gy}},
              (Vert){.color = c, .z = y, .pos = {gx + 0.01f * gpn, gy}},
              (Vert){.color = c, .z = y, .pos = {gx, gy + 0.09f * gpn}});

      if (realmod(ix, 8) == 0 && realmod(iy, 8) == 0) {
        float size =
            fabsf(stb_perlin_fbm_noise3(x * 0.1f, y * 0.1f, 0.0f, 2, 0.5f, 6));
        if (size > 0.35f) {
          geo_tree(&geo, x + gpn * 0.1f, y + gpn * 0.8f,
                   fminf(0.42f, (size - 0.30f) * 4.0f));

          if (realmod(ix, 8 * 3) == 0 && realmod(iy, 8 * 3) == 0 &&
              size < 0.475f) {
            /* we got ourselves a mushroom, son */

            /* hash its position to see if we've generated one here before */
            int ipos[] = {ix, iy};
            uint32_t hash = fnv1_hash(ipos, sizeof(ipos));

            /* TODO: buckets or something to handle hash collisions? idk */
            Mushroom *shroom =
                state.mushrooms + (hash % ARR_LEN(state.mushrooms));

            /* (zero-initalized, so fresh hash) */
            if (shroom->stage == MushroomStage_Undiscovered)
              *shroom = (Mushroom){
                  .stage = MushroomStage_Ripe, .pos = {x, y}, .hash = hash};
            /* in the event of a hash collision: shroomn't */
            else if (shroom->hash != hash) {
              printff(500);
              continue;
            }

            if (onscreen_mush_i < (ARR_LEN(onscreen_mush) - 1))
              onscreen_mush[onscreen_mush_i++] = shroom;

            for (int j = 0; j < 5; j++) {
              float i = j;
              if (shroom->stage == MushroomStage_Collected && i < 2.0f)
                continue;
              if (shroom->stage == MushroomStage_Blasted && i < 3.0f)
                continue;
              float r = i / 5.0f * M_PI * 2.0f + size * 5.77f;
              float mx = x + cosf(r) * (1.0f - i / 5.0f) * 0.21f;
              float my = y + sinf(r) * (1.0f - i / 5.0f) * 0.21f;
              geo_mush(&geo, mx, my, j % 2);
            }
          }
        }
      }
    }

  geo_fireballs(&geo);
  // printff(onscreen_mush_i);

  __builtin_memset(state.todo, 0, sizeof(state.todo));
  if (state.player.man.hp < state.player.man.max_hp)
    state.player.man.hp += 0.0001f;
  quest(&geo, onscreen_mush, dt);

  /* the big cam_apply, for the entire world (almost all assets) */
  cam_apply(geo.vbuf_base, geo.vbuf, aspect);

  /* no cam_apply gets called for this, it it's easier to do in -1..1 */
  { geo_rect(&geo, COLOR_GRASS_BOTTOM, 0.99f, -0.0f, -0.0f, 2.0f, 2.0f); }

  /* until we start sorting all the tris for alpha ... */
  {
    Vert *start = geo.vbuf;
    geo_labels(&geo);
    cam_apply(start, geo.vbuf, aspect);
  }

  /* ghetto text-only cam_apply (just aspect correction so it doesn't stretch)
   */
  {
    Vert *text_start = geo.vbuf;

    for (int i = 0; i < ARR_LEN(state.todo); i++)
      geo_text(&geo, COLOR_TEXT, -0.99f, 0.02f, 0.98f - 0.04f * i,
               state.todo[i], 0.03f);

    for (Vert *t = text_start; t != geo.vbuf; t++) {
      t->pos.x *= aspect;

      // 0..1 is good actually
      t->pos.x = t->pos.x * 2.0f - 1.0f;
      t->pos.y = t->pos.y * 2.0f - 1.0f;
    }
  }

  vbuf(geo.vbuf_base, geo.vbuf - geo.vbuf_base);
  ibuf(geo.ibuf_base, geo.ibuf - geo.ibuf_base);
}
