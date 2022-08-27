#include <stdint.h>
#include <stddef.h>

#define M_PI 3.141592653589793
#define GOLDEN_RATIO (1.618034f)
#define ARR_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define abs(n) (((n) < 0) ? -(n) : (n))

static float round_tof(float x, float n) { return n * (int)(x/n); }
static float fmodf(float x, float n) { return x - n * (int)(x/n); }
static float signf(float x) { return (x < 0.0f) ? -1.0f : 1.0f; }

static int realmod(int x, int p) { return ((x % p) + p) % p; }

static float lerp(float a, float b, float t) {
  return (1.0f-t)*a+t*b;
}
static float lerp_rad(float a, float b, float t) {
  float difference = fmodf(b - a, M_PI*2.0f),
        distance = fmodf(2.0f * difference, M_PI*2.0f) - difference;
  return a + distance * t;
}
static float lerp_round(float max, float a, float b, float t) {
  float difference = fmodf(b - a, max),
        distance = fmodf(2.0f * difference, max) - difference;
  return a + distance * t;
}

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
typedef struct { float x, y, z; } Vec3;
#include "man.h"

#define WASM_EXPORT __attribute__((visibility("default")))
extern float cosf(float);
extern float sinf(float);
extern float randf(void);
extern float sqrtf(float);
extern float printf(float);
extern float atan2f(float, float);
extern void   vbuf(void *ptr, int len);
extern void   ibuf(void *ptr, int len);
extern void netbuf(void *ptr, int len);
extern void netout(void *ptr, int len);

static float fmaxf(float a, float b) { return (a > b) ? a : b; }
static float fminf(float a, float b) { return (a < b) ? a : b; }

typedef struct { float x, y; } Vec2;
static Vec2 vec2_pivot(Vec2 o, Vec2 v, float rads) {
  float oldX = v.x - o.x;
  float oldY = v.y - o.y;
  return (Vec2) {
    .x = o.x + oldX * cosf(rads) - oldY * sinf(rads),
    .y = o.y + oldX * sinf(rads) + oldY * cosf(rads) 
  };
}
static Vec2 add2(Vec2 a, Vec2 b) { return (Vec2) { a.x + b.x,
                                                   a.y + b.y, }; }
static Vec2 mul2_f(Vec2 a, float f) { return (Vec2) { a.x * f,
                                                      a.y * f, }; }
static Vec2 div2_f(Vec2 a, float f) { return (Vec2) { a.x / f,
                                                      a.y / f, }; }
static float dot2(Vec2 a, Vec2 b) { return a.x*b.x + a.y*b.y; }
static float mag2(Vec2 v) { return sqrtf(dot2(v, v)); }
static Vec2 norm2(Vec2 v) { return div2_f(v, mag2(v)); }

typedef union {
  Vec3 xyz;
  struct { float x, y, z, w; };
  float nums[4];
} Vec4;

static Vec3 add3(Vec3 a, Vec3 b) { return (Vec3) { a.x + b.x,
                                                   a.y + b.y,
                                                   a.z + b.z, }; }
static Vec3 sub3(Vec3 a, Vec3 b) { return (Vec3) { a.x - b.x,
                                                   a.y - b.y,
                                                   a.z - b.z, }; }
static Vec3 mul3_f(Vec3 a, float f) { return (Vec3) { a.x * f,
                                                      a.y * f,
                                                      a.z * f, }; }
static Vec3 div3_f(Vec3 a, float f) { return (Vec3) { a.x / f,
                                                      a.y / f,
                                                      a.z / f, }; }
static Vec3 lerp3(Vec3 a, Vec3 b, float t) {
  return add3(mul3_f(a, 1.0f - t), mul3_f(b, t));
}
static float dot3(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static float mag3(Vec3 v) { return sqrtf(dot3(v, v)); }
static Vec3 norm3(Vec3 v) { return div3_f(v, mag3(v)); }
static Vec3 cross3(Vec3 a, Vec3 b) {
  return (Vec3){(a.y * b.z) - (a.z * b.y),
                (a.z * b.x) - (a.x * b.z),
                (a.x * b.y) - (a.y * b.x)};
}

typedef struct { float nums[4][4]; } Mat4;

static Mat4 look_at4x4(Vec3 eye, Vec3 focus, Vec3 up) {
  Vec3 eye_dir = sub3(focus, eye);
  Vec3 R2 = norm3(eye_dir);

  Vec3 R0 = norm3(cross3(up, R2));
  Vec3 R1 = cross3(R2, R0);

  Vec3 neg_eye = mul3_f(eye, -1.0f);

  float D0 = dot3(R0, neg_eye);
  float D1 = dot3(R1, neg_eye);
  float D2 = dot3(R2, neg_eye);

  return (Mat4) {{
    { R0.x, R1.x, R2.x, 0.0f },
    { R0.y, R1.y, R2.y, 0.0f },
    { R0.z, R1.z, R2.z, 0.0f },
    {   D0,   D1,   D2, 1.0f }
  }};
}

static Vec4 mul4x44(Mat4 m, Vec4 v) {
  Vec4 res;
  for(int x = 0; x < 4; ++x) {
    float sum = 0;
    for(int y = 0; y < 4; ++y)
      sum += m.nums[y][x] * v.nums[y];

    res.nums[x] = sum;
  }
  return res;
}

typedef struct { float r, g, b, a; } Color;
WASM_EXPORT Color color_picked = { 0.0f, 0.0f, 0.0f, 1.0f };
#define COLOR_BLACK         ((Color) { 0.00f, 0.00f, 0.00f, 1.00f })
#define COLOR_WHITE         ((Color) { 1.00f, 1.00f, 1.00f, 1.00f })
#define COLOR_BEIGE         ((Color) { 0.72f, 0.64f, 0.53f, 1.00f })
#define COLOR_BROWN         ((Color) { 0.46f, 0.27f, 0.18f, 1.00f })
#define COLOR_DARKBROWN     ((Color) { 0.30f, 0.25f, 0.18f, 1.00f })
#define COLOR_DARKERBROWN   ((Color) { 0.25f, 0.20f, 0.13f, 1.00f })
#define COLOR_BLUE          ((Color) { 0.00f, 0.47f, 0.95f, 1.00f })
#define COLOR_RED           ((Color) { 0.90f, 0.16f, 0.22f, 1.00f })
#define COLOR_MAROON        ((Color) { 0.75f, 0.13f, 0.22f, 1.00f })
#define COLOR_DARKMAROON    ((Color) { 0.55f, 0.03f, 0.12f, 1.00f })
#define COLOR_GREEN         ((Color) { 0.00f, 0.89f, 0.19f, 1.00f })
#define COLOR_LIGHTGREY     ((Color) { 0.78f, 0.78f, 0.78f, 1.00f })
#define COLOR_LIGHTISHGREY  ((Color) { 0.68f, 0.68f, 0.68f, 1.00f })
#define COLOR_GREY          ((Color) { 0.51f, 0.51f, 0.51f, 1.00f })
#define COLOR_DARKGREY      ((Color) { 0.31f, 0.31f, 0.31f, 1.00f })
#define COLOR_YELLOW        ((Color) { 0.99f, 0.98f, 0.00f, 1.00f })

#define COLOR_TEXT          ((Color) { 0.95f, 0.75f, 0.32f, 1.00f })

#define COLOR_SLOTCOLOR     ((Color) { 0.42f, 0.40f, 0.39f, 1.00f })
#define COLOR_DARKSLOTCOLOR ((Color) { 0.32f, 0.30f, 0.29f, 1.00f })
#define COLOR_TREEBORDER    ((Color) { 0.00f, 0.42f, 0.13f, 1.00f })
#define COLOR_TREEGREEN     ((Color) { 0.00f, 0.46f, 0.17f, 1.00f })
#define COLOR_TREEGREEN1    ((Color) { 0.04f, 0.50f, 0.21f, 1.00f })
#define COLOR_TREEGREEN2    ((Color) { 0.08f, 0.54f, 0.25f, 1.00f })
#define COLOR_TREEGREEN3    ((Color) { 0.12f, 0.58f, 0.29f, 1.00f })
#define COLOR_BARK          ((Color) { 0.33f, 0.24f, 0.16f, 1.00f })
#define COLOR_GRASS_TOP     ((Color) { 0.15f, 0.61f, 0.33f, 1.00f })
#define COLOR_FORESTSHADOW  ((Color) { 0.01f, 0.46f, 0.27f, 1.00f })
#define COLOR_GRASS_BOTTOM  ((Color) { 0.05f, 0.51f, 0.29f, 1.00f })

typedef struct { Vec2 pos; float z, letter; Color color; } Vert;

typedef struct {
  Vec2 pos;
  float anim_prog, anim_damp;
  float dir;
} Man;

typedef struct {
  uint32_t id;
  Man man; /* probably needa couple to tween */

  /* maybe last time we got an update about this person?
   * so we can garbage collect? (when we actually filter by location) */
} Other;

typedef struct {
  uint32_t id;
  Man man;
} NetMsg;

static struct {

  /* graphics */
  Vert vbuf[1 << 16];
  uint16_t ibuf[1 << 17];
  float letter_width_buf[128];
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
    struct { int x, y; } mouse_start;
    struct { float x, y; } cam_start;
  } drag;
  uint8_t keys_down[255];
  struct { Vec2 vel; Man man; } player;

} state = {0};

/* a collection of geometry (vertices, indices) you can write into */
typedef struct {
  Vert *vbuf;
  Vert *vbuf_base;
  size_t vbuf_max;

  uint16_t *ibuf;
  uint16_t *ibuf_base;
  size_t ibuf_max;
} Geo;

/* note: doesn't un-zoom (should it?) */
static Vec2 px_to_world_space(int x, int y) {
  float aspect = (float)state.height / (float)state.width;
  Vec2 ret = { x, y };
  ret.x /= state.width;
  ret.y /= state.height;
  
  ret.x /= aspect;
  return ret;
}

static void geo_apply_cam(Geo *geo, float width, float height) {
  float min_z = -1.0f;
  float max_z =  1.0f;

  for (Vert *p = geo->vbuf_base; p != geo->vbuf; p++)
    min_z = (p->z < min_z) ? p->z : min_z,
    max_z = (p->z > max_z) ? p->z : max_z;

  /* more width: show more
     more height: zoom in */
  float aspect = (float)height / (float)width;
  for (Vert *p = geo->vbuf_base; p != geo->vbuf; p++) {
    p->pos.x += state.cam.x;
    p->pos.y += state.cam.y;

    p->pos.x *= aspect;
    p->z = ((p->z - min_z) / (max_z - min_z))*2.0f - 1.0f;
    p->z *= 0.99999f;

    // zoom
    p->pos.x /= state.zoom;
    p->pos.y /= state.zoom;

    // 0..1 is good actually
    p->pos.x = p->pos.x * 2.0f - 1.0f;
    p->pos.y = p->pos.y * 2.0f - 1.0f;
  }
}

static void geo_ibuf_push(Geo *geo, uint16_t a, uint16_t b, uint16_t c) {
  if ((geo->ibuf - geo->ibuf_base) < geo->ibuf_max)
    *geo->ibuf++ = a, *geo->ibuf++ = b, *geo->ibuf++ = c;
  else
    printf(500);
}

static void geo_vbuf_push(Geo *geo, Vert v) {
  if ((geo->vbuf - geo->vbuf_base) < geo->vbuf_max)
    *geo->vbuf++ = v;
  else
    printf(500);
}

static void geo_quad(Geo *geo, Vert tl, Vert tr, Vert br, Vert bl) {
  int i = geo->vbuf - geo->vbuf_base;

  geo_vbuf_push(geo, tl);
  geo_vbuf_push(geo, tr);
  geo_vbuf_push(geo, br);
  geo_vbuf_push(geo, bl);

  geo_ibuf_push(geo, i+0, i+1, i+2);
  geo_ibuf_push(geo, i+2, i+1, i+3);
}

static void geo_tri(Geo *geo, Vert a, Vert b, Vert c) {
  int i = geo->vbuf - geo->vbuf_base;

  geo_vbuf_push(geo, a);
  geo_vbuf_push(geo, b);
  geo_vbuf_push(geo, c);

  geo_ibuf_push(geo, i+0, i+1, i+2);
}

static void geo_ngon(Geo *geo, Color c, float z, float x, float y, float r, float n) {
  int i;
  int center = geo->vbuf - geo->vbuf_base;
  geo_vbuf_push(geo, (Vert) { .pos = { x, y }, .z=z, .color = c });

  #define CIRC_VERT(n) ((Vert) {   \
      .pos = { x + cosf(a*(n))*r,    \
               y + sinf(a*(n))*r  }, \
      .z = z,                      \
      .color = c                   \
    })

  float a = (M_PI*2)/n;
  int start = geo->vbuf - geo->vbuf_base;
  geo_vbuf_push(geo, CIRC_VERT(1));

  for (int ni = 2; ni < (n+1); ni++) {
    i = geo->vbuf - geo->vbuf_base;
    geo_vbuf_push(geo, CIRC_VERT(ni));
    geo_ibuf_push(geo, i, center, i-1);
  }

  geo_ibuf_push(geo, start, center, i);
}
static void geo_8gon(Geo *geo, Color c, float z, float x, float y, float r) {
  geo_ngon(geo, c, z, x, y, r, 8.0f);
}

static void geo_line(Geo *geo, Color c, float z, Vec2 beg, Vec2 end, float thickness) {
  float dx = end.x - beg.x;
  float dy = end.y - beg.y;

  // https://stackoverflow.com/questions/begend436beg4/how-do-i-calculate-the-normal-vector-of-a-line-segment

  float nx = -dy;
  float ny = dx;

  float tlen = sqrtf(nx * nx + ny * ny) / (thickness * 0.5);
  if (tlen < 0.00000001f) return;
  float tx = nx / tlen;
  float ty = ny / tlen;

  geo_quad(geo,
    (Vert) { .pos = { beg.x + tx, beg.y + ty }, .z=z, .color = c },
    (Vert) { .pos = { beg.x - tx, beg.y - ty }, .z=z, .color = c },
    (Vert) { .pos = { end.x + tx, end.y + ty }, .z=z, .color = c },
    (Vert) { .pos = { end.x - tx, end.y - ty }, .z=z, .color = c }
  );
}

static void geo_rect(Geo *geo, Color c, float z, float x, float y, float w, float h) {
  geo_line(geo, c, z, (Vec2) { .x = x-w/2, .y = y},
                      (Vec2) { .x = x+w/2, .y = y}, h);
}

static void geo_text(Geo *geo, Color c, float z, float x, float y, char *str, float size) {
  do {

    geo_quad(geo,
      (Vert) { .pos = { x + size, y - size }, .z=z, .color=c, .letter=*str + 0.00f },
      (Vert) { .pos = { x       , y - size }, .z=z, .color=c, .letter=*str + 0.25f },
      (Vert) { .pos = { x + size, y        }, .z=z, .color=c, .letter=*str + 0.50f },
      (Vert) { .pos = { x       , y        }, .z=z, .color=c, .letter=*str + 0.75f }
    );

    x += size * state.letter_width_buf[*str];

    z -= 0.00001f;
  } while(*++str);
}

static void geo_tree(Geo *geo, float x, float _y, float size) {
  Vert *start = geo->vbuf;
  float w = 0.8f, h = GOLDEN_RATIO, r = 0.4f;
  float y = _y - 0.2f;

  geo_ngon(geo, COLOR_BARK, y, x,       y + r, r, 32);
  geo_rect(geo, COLOR_BARK, y, x, h/2 + y + r, w, h);

  geo_8gon(geo, COLOR_TREEGREEN , y - 1.8f, x + 0.80f, y + 2.2f, 0.8f);
  geo_8gon(geo, COLOR_TREEGREEN1, y - 1.8f, x + 0.16f, y + 3.0f, 1.0f);
  geo_8gon(geo, COLOR_TREEGREEN2, y - 1.8f, x - 0.80f, y + 2.5f, 0.9f);
  geo_8gon(geo, COLOR_TREEGREEN3, y - 1.8f, x - 0.16f, y + 2.0f, 0.8f);

  geo_8gon(geo, COLOR_TREEBORDER, y, x + 0.80f, y + 2.2f, 0.8f+0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x + 0.16f, y + 3.0f, 1.0f+0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x - 0.80f, y + 2.5f, 0.9f+0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x - 0.16f, y + 2.0f, 0.8f+0.1f);

  float sr = 1.22f;
  geo_8gon(geo, COLOR_FORESTSHADOW, y + sr, x, y + r, sr);

  for (Vert *v = start; v != geo->vbuf; v++)
    v->pos.x += (size - 1.0f) * (v->pos.x -  x),
    v->pos.y += (size - 1.0f) * (v->pos.y - _y),
    v->    z += (size - 1.0f) * (v->    z - _y);
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
    if (other->id) continue;

    other->id = msg->id;
    other->man = msg->man;
    return;
  }
}

/* you can always rewrite netcode to make an existing game secure.
 * .. what you can't do ...
 * is get back time you spent securing a game nobody will ever play. */

WASM_EXPORT float *letter_width_buf(void) { return state.letter_width_buf; }
WASM_EXPORT void init(void) {
  netbuf(state.netbuf, ARR_LEN(state.netbuf));
  state.id = randf() * (float)(UINT32_MAX); // TODO: precision?

  state.zoom = 5.0f;
  vbuf(state.vbuf, ARR_LEN(state.vbuf));
  ibuf(state.ibuf, ARR_LEN(state.ibuf));

  man_frames_fill(&state.mf);
  typedef struct { ManPartKind lhs, rhs; } PartPair;
  PartPair mirror[] = {
    { ManPartKind_Knee_L , ManPartKind_Knee_R  },
    { ManPartKind_Sole_L , ManPartKind_Sole_R  },
    { ManPartKind_Toe_L  , ManPartKind_Toe_R   },
    { ManPartKind_Elbow_L, ManPartKind_Elbow_R },
    { ManPartKind_Hand_L , ManPartKind_Hand_R  },
  };
  ManPartKind no[] = {
    ManPartKind_Elbow_L, ManPartKind_Elbow_R,
    ManPartKind_Hand_L , ManPartKind_Hand_R ,
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
      state.mf.frames[5+j].pos[lhs].y =  state.mf.frames[1+j].pos[rhs].y;
      state.mf.frames[5+j].pos[lhs].z =  state.mf.frames[1+j].pos[rhs].z;
      state.mf.frames[5+j].pos[rhs].y =  state.mf.frames[1+j].pos[lhs].y;
      state.mf.frames[5+j].pos[rhs].z =  state.mf.frames[1+j].pos[lhs].z;
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
        Vec2 p = px_to_world_space(
          x - state.drag.mouse_start.x,
          y - state.drag.mouse_start.y 
        );
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
  #define X_SIZE (state.zoom * (float)state. width)
  #define Y_SIZE (state.zoom * (float)state.height)
  float x_size_before = X_SIZE;
  float y_size_before = Y_SIZE;

  /* --- apply zoom --- */
  state.zoom *= t;

  Vec2 p = px_to_world_space(
    (x_size_before - X_SIZE)/2.0f,
    (y_size_before - Y_SIZE)/2.0f
  );
  state.cam.x -= p.x;
  state.cam.y -= p.y;

  #undef X_SIZE
  #undef Y_SIZE
}

static void man_anim(Man *man, float dt, Vec2 vel) {
  uint8_t going = dot2(vel, vel) > 0.00001f;
  man->anim_damp = lerp(man->anim_damp, going, dt * 0.15f);
  man->anim_prog += 0.14f*man->anim_damp*dt;
  int anim_len = ARR_LEN(state.mf.frames);
  man->anim_prog = lerp_round(anim_len,
    man->anim_prog, going*man->anim_prog, dt*0.05f); 
  man->anim_prog = fmodf(man->anim_prog, anim_len);

  man->dir = lerp_rad(man->dir, atan2f(-vel.y, -vel.x), going*dt*0.1f);
}

static Vec2 man_pos(Man *man, ManPartKind mpk) {
  float q = man->anim_prog;
  float t = q - round_tof(q, 1.0f);

  int len = ARR_LEN(state.mf.frames);
  int i = (int)(q+0  ) % len;
  int n = (int)(q+0+1) % len;

  Vec3 a = state.mf.frames[i].pos[mpk];
  Vec3 b = state.mf.frames[n].pos[mpk];
  Vec4 p = { .x = lerp(a.x, b.x, t) * 1.35f,
             .y = lerp(a.y, b.y, t),
             .z = lerp(a.z, b.z, t),
             .w = 1.0f };

  float mx = cosf(man->dir);
  float my = sinf(man->dir);
  Mat4 m = look_at4x4((Vec3) { mx, my, 0.75f },
                      (Vec3) {  0,  0, 0     },
                      (Vec3) {  0,  0, 1     });
  p = mul4x44(m, p);
  return (Vec2) { p.x * 0.5f + man->pos.x,
                  p.y * 0.5f + man->pos.y };
}

static void geo_man(Geo *geo, Man *man, uint32_t id) {
  Vec3 skin_color3 = lerp3(
    (Vec3) { 0.20f, 0.25f, 0.43f },
    (Vec3) { 0.26f, 0.19f, 0.43f },
    (float)id/(float)(UINT32_MAX));
  Color skin_color = {
    .r = skin_color3.x,
    .g = skin_color3.y,
    .b = skin_color3.z,
    .a = 1.0f,
  };

  float z = man_pos(man, ManPartKind_Toe_R).y - 0.1f;

  Vec2 head = man_pos(man, ManPartKind_Head);
  geo_ngon(geo, skin_color, z, head.x, head.y, 0.08f, 32);

  typedef struct { ManPartKind lhs, rhs; } PartPair;
  PartPair pp[] = {
    { ManPartKind_Neck,    ManPartKind_Pelvis  },
    { ManPartKind_Neck,    ManPartKind_Elbow_R },
    { ManPartKind_Neck,    ManPartKind_Elbow_L },
    { ManPartKind_Elbow_R, ManPartKind_Hand_R  },
    { ManPartKind_Elbow_L, ManPartKind_Hand_L  },
    { ManPartKind_Pelvis,  ManPartKind_Knee_R  },
    { ManPartKind_Pelvis,  ManPartKind_Knee_L  },
    { ManPartKind_Knee_R,  ManPartKind_Sole_R  },
    { ManPartKind_Knee_L,  ManPartKind_Sole_L  },
    { ManPartKind_Sole_R,  ManPartKind_Toe_R   },
    { ManPartKind_Sole_L,  ManPartKind_Toe_L   },
  };

  float thickness = 0.0125f;
  for (ManPartKind i = 0; i < ManPartKind_COUNT; i++) {
    Vec2 pos = man_pos(man, i);
    geo_8gon(geo, skin_color, z, pos.x, pos.y, thickness);
  }

  for (int i = 0; i < ARR_LEN(pp); i++)
    geo_line(geo, skin_color, z,
      man_pos(man, pp[i].lhs),
      man_pos(man, pp[i].rhs),
      thickness*2.0f);
}

WASM_EXPORT void frame(int width, int height, double _dt) {
  float dt = _dt / (1000.0/60.0);

  /* tell everyone where we are every 0.2 seconds */
  state.time_since_netout += dt;
  if (state.time_since_netout > 0.2f) {
    state.time_since_netout = 0.0f;

    NetMsg msg = { state.id, state.player.man };
    netout(&msg, sizeof(msg));
  }

  /* passing this down to geo fns is annoying */
  state. width =  width;
  state.height = height;

  Geo geo = {
    .vbuf      = state.vbuf,
    .vbuf_base = state.vbuf,
    .vbuf_max  = ARR_LEN(state.vbuf),
    .ibuf      = state.ibuf,
    .ibuf_base = state.ibuf,
    .ibuf_max  = ARR_LEN(state.ibuf),
  };
  __builtin_memset(state.ibuf, 0, sizeof(state.ibuf));

  float aspect = (float)height/(float)width;

  // geo_rect(&geo, COLOR_GREEN, -0.1f, 0.1f,0.1f, 0.2f,0.2f);
  // geo_rect(&geo, COLOR_RED  , -0.2f, 0.0f,0.0f, 0.2f,0.2f);
  // geo_rect(&geo, COLOR_BLUE , -0.3f, 0.2f,0.2f, 0.2f,0.2f);

  Vec2 move = {};
  if (state.keys_down['w']) move.y += 1;
  if (state.keys_down['s']) move.y -= 1;
  if (state.keys_down['a']) move.x -= 1;
  if (state.keys_down['d']) move.x += 1;
  if (dot2(move, move) > 0.0f) move = norm2(move);

  state.player.vel = add2(state.player.vel, mul2_f(move, 0.002f));
  state.player.vel = mul2_f(state.player.vel, 0.85f);

  if (dot2(state.player.vel, state.player.vel) > 0.00008f)
    state.player.man.pos = add2(state.player.man.pos,
                                mul2_f(norm2(state.player.vel),
                                       0.013f*state.player.man.anim_damp));

  man_anim(&state.player.man, dt, state.player.vel);
  geo_man(&geo, &state.player.man, state.id);

  {
    static Man pacing_man = {0};
    static float t = 0.0f;
    t += dt;

    Vec2 vel = {       cosf(t*0.01f+M_PI/2),
                       sinf(t*0.01f+M_PI/2) };
    Vec2 pos = { 1.00f*cosf(t*0.01f       ),
                 1.00f*sinf(t*0.01f       ) };

    pacing_man.pos = pos;
    man_anim(&pacing_man, dt, vel);

    geo_man(&geo, &pacing_man, 0);
  }

  for (int i = 0; i < ARR_LEN(state.others); i++) {
    Other *other = state.others + i;
    if (other->id)
      geo_man(&geo, &other->man, other->id);
  }

  /* lerp cam towards player */
  {
    Vec2 halsc = px_to_world_space(
      (state.zoom * (float)state. width)/2.0f,
      (state.zoom * (float)state.height)/2.0f
    );
    Vec2 head = man_pos(&state.player.man, ManPartKind_Head);
    Vec2 p = mul2_f(add2(state.player.man.pos, head), 0.5f);
    state.cam = (Vec2) { lerp(state.cam.x, -p.x + halsc.x, dt*0.04f),
                         lerp(state.cam.y, -p.y + halsc.y, dt*0.04f) };
  }

  // geo_man(&geo, dt, 0.3f*cosf(dt*0.001f+M_PI/2),
  //                   0.3f*sinf(dt*0.001f+M_PI/2));
    

  float pad = 2.5f;
  Vec2 min = { -round_tof(state.cam.x, 0.1f) - pad   + 1.0f,
               -round_tof(state.cam.y, 0.1f) - pad   - 2.0f };
  Vec2 max = { min.x + state.zoom * aspect   + pad*2,
               min.y + state.zoom            + pad*2 + 4.0f};

  for (  float _x = min.x; _x < max.x; _x += 0.1f)
    for (float _y = min.y; _y < max.y; _y += 0.1f) {
      /* the 0.000001f bias prevents -1.0f from getting trunced to 0 */ 
      int ix = (_x + 0.0001f*signf(_x))/0.1f;
      int iy = (_y + 0.0001f*signf(_y))/0.1f;

      /* the 0.05f bias centers things in their tiles */
      float x = _x + 0.05f;
      float y = _y + 0.05f;

      float gpn = fabsf(stb_perlin_fbm_noise3(x*0.4f,y*0.4f,0.0f, 2,0.5f,6))*3;
      gpn = fminf(gpn, 2.0f);
      // geo_rect(&geo, COLOR_GRASS_TOP, y, x,y, size,size);
      Color c = COLOR_GRASS_TOP;
      float gx = realmod(iy, 2) ? x + 0.05f : x;
      float gy = y;
      geo_tri(&geo, (Vert) { .color=c, .z=y, .pos={ gx-0.01f*gpn, gy           } },
                    (Vert) { .color=c, .z=y, .pos={ gx+0.01f*gpn, gy           } },
                    (Vert) { .color=c, .z=y, .pos={ gx          , gy+0.09f*gpn } });

      if (realmod(ix, 8) == 0 &&
          realmod(iy, 8) == 0 ) {
        float size = fabsf(stb_perlin_fbm_noise3(x*0.1f,y*0.1f,0.0f, 2,0.5f,6));
        if (size > 0.35f)
          geo_tree(&geo, x+gpn*0.1f,
                         y+gpn*0.8f, fminf(0.42f, (size-0.30f)*4.0f));
      }
    }

  // EX: adding nametag to player
  // geo_text(&geo, COLOR_RED, state.player.man.pos.y-1.0f,
  //                           state.player.man.pos.x,
  //                           state.player.man.pos.y,
  //                           "ced",
  //                           0.1f);

  geo_apply_cam(&geo, width, height);

  geo_rect(&geo, COLOR_GRASS_BOTTOM, 0.99f, -0.0f,-0.0f, 2.0f,2.0f);

  /* ghetto text-only apply_cam (just aspect correction) */
  Vert *text_start = geo.vbuf;
  geo_text(&geo, COLOR_TEXT,-0.99f, 0.0f, 1.0f, "be very fuckin scared", 0.03f);
  for (Vert *t = text_start; t != geo.vbuf; t++) {
    t->pos.x *= aspect;

    // 0..1 is good actually
    t->pos.x = t->pos.x * 2.0f - 1.0f;
    t->pos.y = t->pos.y * 2.0f - 1.0f;
  }

  vbuf(geo.vbuf_base, geo.vbuf - geo.vbuf_base);
  ibuf(geo.ibuf_base, geo.ibuf - geo.ibuf_base);
}
