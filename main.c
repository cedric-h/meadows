#define M_PI 3.141592653589793
#define ARR_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define WASM_EXPORT __attribute__((visibility("default")))
extern float cosf(float);
extern float sinf(float);
extern void vbuf(void *ptr, int len);

typedef struct { float x, y; } Vec2;
static Vec2 vec2_unit_from_rads(float rads) {
  return (Vec2) { .x = cosf(rads), .y = sinf(rads) };
}

typedef struct { float r, g, b, a; } Color;
#define COLOR_RED  ((Color) { 1.0f, 0.0f, 0.0f, 1.0f })
#define COLOR_BLUE ((Color) { 0.0f, 0.0f, 1.0f, 1.0f })

typedef struct { Vec2 pos; float _pad0, _pad1; Color color; } Vert;

static struct {
  Vert vbuf[6];
} state = {0};

void init(void) {
  vbuf(state.vbuf, ARR_LEN(state.vbuf));
}

void frame(int width, int height, float dt) {
  Vert *out = state.vbuf;

  float rad = dt*0.0008;
  float rad_inc = M_PI*2.0f / 3.0f;
  *out++ = (Vert) { .pos = vec2_unit_from_rads(rad += rad_inc),
                    .color = COLOR_RED };
  *out++ = (Vert) { .pos = vec2_unit_from_rads(rad += rad_inc),
                    .color = COLOR_RED };
  *out++ = (Vert) { .pos = vec2_unit_from_rads(rad += rad_inc),
                    .color = COLOR_RED };

  rad = dt*0.0007 + rad_inc/2.0f;
  *out++ = (Vert) { .pos = vec2_unit_from_rads(rad += rad_inc),
                    .color = COLOR_BLUE };
  *out++ = (Vert) { .pos = vec2_unit_from_rads(rad += rad_inc),
                    .color = COLOR_BLUE };
  *out++ = (Vert) { .pos = vec2_unit_from_rads(rad += rad_inc),
                    .color = COLOR_BLUE };

  /* more width: show more
     more height: zoom in */
  float aspect = (float)height / (float)width;
  for (Vert *p = state.vbuf; p != out; p++)
    p->pos.x *= aspect;
}
