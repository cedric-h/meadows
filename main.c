#include <stdint.h>

#define M_PI 3.141592653589793
#define GOLDEN_RATIO (1.618034f)
#define ARR_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define WASM_EXPORT __attribute__((visibility("default")))
extern float cosf(float);
extern float sinf(float);
extern float sqrtf(float);
extern float printf(float);
extern void vbuf(void *ptr, int len);
extern void ibuf(void *ptr, int len);

typedef struct { float x, y; } Vec2;
static Vec2 vec2_unit_from_rads(float rads) {
  return (Vec2) { .x = cosf(rads), .y = sinf(rads) };
}
static Vec2 vec2_pivot(Vec2 o, Vec2 v, float rads) {
  float oldX = v.x - o.x;
  float oldY = v.y - o.y;
  return (Vec2) {
    .x = o.x + oldX * cosf(rads) - oldY * sinf(rads),
    .y = o.y + oldX * sinf(rads) + oldY * cosf(rads) 
  };
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

#define COLOR_SLOTCOLOR     ((Color) { 0.42f, 0.40f, 0.39f, 1.00f })
#define COLOR_DARKSLOTCOLOR ((Color) { 0.32f, 0.30f, 0.29f, 1.00f })
#define COLOR_FORESTSHADOW  ((Color) { 0.07f, 0.55f, 0.27f, 1.00f })
#define COLOR_TREEBORDER    ((Color) { 0.00f, 0.42f, 0.13f, 1.00f })
#define COLOR_TREEGREEN     ((Color) { 0.00f, 0.46f, 0.17f, 1.00f })
#define COLOR_TREEGREEN1    ((Color) { 0.04f, 0.50f, 0.21f, 1.00f })
#define COLOR_TREEGREEN2    ((Color) { 0.08f, 0.54f, 0.25f, 1.00f })
#define COLOR_TREEGREEN3    ((Color) { 0.12f, 0.58f, 0.29f, 1.00f })
#define COLOR_BARK          ((Color) { 0.37f, 0.32f, 0.29f, 1.00f })

typedef struct { Vec2 pos; float z, _pad0; Color color; } Vert;

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
    p->pos.x *= aspect;
    p->z = ((p->z - min_z) / (max_z - min_z))*2.0f - 1.0f;
    p->z *= 0.99999f;

    p->pos.x /= 5.0f;
    p->pos.y /= 5.0f;

    /* actually being centered is good */
    // p->pos.x = 2.0f*p->pos.x - 1.0f;
    // p->pos.y = 2.0f*p->pos.y - 1.0f;
  }
}

static void geo_quad(Geo *geo, Vert tl, Vert tr, Vert br, Vert bl) {
  int i = geo->vbuf - geo->vbuf_base;

  *geo->vbuf++ = tl;
  *geo->vbuf++ = tr;
  *geo->vbuf++ = br;
  *geo->vbuf++ = bl;

  *geo->ibuf++ = i+0, *geo->ibuf++ = i+1, *geo->ibuf++ = i+2;
  *geo->ibuf++ = i+2, *geo->ibuf++ = i+1, *geo->ibuf++ = i+3;
}

static void geo_tri(Geo *geo, Vert a, Vert b, Vert c) {
  int i = geo->vbuf - geo->vbuf_base;

  *geo->vbuf++ = a;
  *geo->vbuf++ = b;
  *geo->vbuf++ = c;

  *geo->ibuf++ = i+0, *geo->ibuf++ = i+1, *geo->ibuf++ = i+2;
}

static void geo_ngon(Geo *geo, Color c, float z, float x, float y, float r, float n) {
  int i;
  int center = geo->vbuf - geo->vbuf_base;
  *geo->vbuf++ = (Vert) { .pos = { x, y }, .z=z, .color = c };

  float a = (M_PI*2)/n;
  int start = geo->vbuf - geo->vbuf_base;
  *geo->vbuf++ = (Vert) { .pos = { x + cosf(a*1)*r, y + sinf(a*1)*r }, .z=z, .color = c };

  for (int ni = 2; ni < (n+1); ni++) {
    i = geo->vbuf - geo->vbuf_base;
    *geo->vbuf++ = (Vert) { .pos = { x + cosf(a*ni)*r, y + sinf(a*ni)*r }, .z=z, .color = c };
    *geo->ibuf++ = i, *geo->ibuf++ = center, *geo->ibuf++ = i-1;
  }

  *geo->ibuf++ = start, *geo->ibuf++ = center, *geo->ibuf++ = i;
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

static void geo_tree(Geo *geo, float x, float y) {
  float w = 0.8f, h = GOLDEN_RATIO, r = 0.4f;
  geo_ngon(geo, COLOR_BARK, y, x,       y + r, r, 32);
  geo_rect(geo, COLOR_BARK, y, x, h/2 + y + r, w, h);

  geo_8gon(geo, COLOR_TREEGREEN , y - 1.1f, x + 0.80f, y + 2.2f, 0.8f);
  geo_8gon(geo, COLOR_TREEGREEN1, y - 1.1f, x + 0.16f, y + 3.0f, 1.0f);
  geo_8gon(geo, COLOR_TREEGREEN2, y - 1.1f, x - 0.80f, y + 2.5f, 0.9f);
  geo_8gon(geo, COLOR_TREEGREEN3, y - 1.1f, x - 0.16f, y + 2.0f, 0.8f);

  geo_8gon(geo, COLOR_TREEBORDER, y, x + 0.80f, y + 2.2f, 0.8f+0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x + 0.16f, y + 3.0f, 1.0f+0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x - 0.80f, y + 2.5f, 0.9f+0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x - 0.16f, y + 2.0f, 0.8f+0.1f);

  float sr = 0.92f;
  geo_8gon(geo, COLOR_FORESTSHADOW, y + sr, x, y + r, sr);
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

  float aspect = (float)width/(float)height;
  Color bg = { 0.15f, 0.61f, 0.33f, 1.00f };
  geo_rect(&geo, bg, 2.0f, 0.0f,0.0f, 10.0f*aspect, 10.0f);// 1.0f*aspect,2.0f);

  // geo_rect(&geo, COLOR_GREEN, -0.1f, 0.1f,0.1f, 0.2f,0.2f);
  // geo_rect(&geo, COLOR_RED  , -0.2f, 0.0f,0.0f, 0.2f,0.2f);
  // geo_rect(&geo, COLOR_BLUE , -0.3f, 0.2f,0.2f, 0.2f,0.2f);

  geo_tree(&geo, 0.0f, 0.0f);

  geo_apply_cam(&geo, width, height);
}
