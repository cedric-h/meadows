#include <stdint.h>

#define M_PI 3.141592653589793
#define ARR_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define WASM_EXPORT __attribute__((visibility("default")))
extern float cosf(float);
extern float sinf(float);
extern void vbuf(void *ptr, int len);
extern void ibuf(void *ptr, int len);

typedef struct { float x, y; } Vec2;
// static Vec2 vec2_unit_from_rads(float rads) {
//   return (Vec2) { .x = cosf(rads), .y = sinf(rads) };
// }
static Vec2 vec2_pivot(Vec2 o, Vec2 v, float rads) {
  float oldX = v.x - o.x;
  float oldY = v.y - o.y;
  return (Vec2) {
    .x = o.x + oldX * cosf(rads) - oldY * sinf(rads),
    .y = o.y + oldX * sinf(rads) + oldY * cosf(rads) 
  };
}

typedef struct { float r, g, b, a; } Color;
#define COLOR_RED  ((Color) { 1.0f, 0.0f, 0.0f, 1.0f })
#define COLOR_BLUE ((Color) { 0.0f, 0.0f, 1.0f, 1.0f })

typedef struct { Vec2 pos; float _pad0, _pad1; Color color; } Vert;

static struct {
  Vert vbuf[1 << 10];
  uint16_t ibuf[1 << 11];
} state = {0};

/* a collection of geometry (vertices, indices) you can write into */
typedef struct {
  Vert *vbuf;
  Vert *vbuf_base;

  uint16_t *ibuf;
  uint16_t *ibuf_base;
} Geo;

static void geo_aspect_fix(Geo *geo, float width, float height) {
  /* more width: show more
     more height: zoom in */
  float aspect = (float)height / (float)width;
  for (Vert *p = geo->vbuf_base; p != geo->vbuf; p++)
    p->pos.x *= aspect;
}

static void geo_rect(Geo *geo, float x, float y, float w, float h, Color c) {
  int i = geo->vbuf - geo->vbuf_base;

  float min_x = x - w/2;
  float min_y = y - h/2;
  *geo->vbuf++ = (Vert) { .pos = { min_x  , min_y   }, .color = c };
  *geo->vbuf++ = (Vert) { .pos = { min_x+w, min_y   }, .color = c };
  *geo->vbuf++ = (Vert) { .pos = { min_x+w, min_y+h }, .color = c };
  *geo->vbuf++ = (Vert) { .pos = { min_x  , min_y+h }, .color = c };

  *geo->ibuf++ = i+3; *geo->ibuf++ = i+2; *geo->ibuf++ = i+1;
  *geo->ibuf++ = i+3; *geo->ibuf++ = i+1; *geo->ibuf++ = i+0;
}

WASM_EXPORT void init(void) {
  vbuf(state.vbuf, ARR_LEN(state.vbuf));
  ibuf(state.ibuf, ARR_LEN(state.ibuf));
}

WASM_EXPORT void frame(int width, int height, float dt) {
  Geo geo = {
    .vbuf      = state.vbuf,
    .vbuf_base = state.vbuf,
    .ibuf      = state.ibuf,
    .ibuf_base = state.ibuf,
  };

  Vert *start = geo.vbuf;
  for (int i = 0; i < 10; i++) {
    Vert *start = geo.vbuf;

    float size = 0.05f;
    float r = (float)i / 10.0f * (M_PI*2);
    Vec2 p = { 0.0f, 0.8f };
    p = vec2_pivot((Vec2) { 0.0f }, p, r);
    geo_rect(&geo, p.x, p.y, size, size, COLOR_RED);

    for (; start != geo.vbuf; start++)
      start->pos = vec2_pivot(p, start->pos, dt*0.008);
  }
  for (; start != geo.vbuf; start++)
    start->pos = vec2_pivot((Vec2) {0}, start->pos, dt*0.0008);

  geo_aspect_fix(&geo, width, height);
}
