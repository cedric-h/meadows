#define M_PI 3.141592653589793

#define WASM_EXPORT __attribute__((visibility("default")))
extern float cosf(float);
extern float sinf(float);

typedef struct { float x, y; } Pos;
static struct {
  Pos vbuf[3];
} state = {0};

void init(void) {}

Pos *vbuf(void) { return state.vbuf; }

void frame(int width, int height, float dt) {
  Pos *out = state.vbuf;

  *out++ = (Pos) { .x = 0.5f*cosf(dt*0.0008 + (M_PI*2.0f / 3.0f)*1.0f),
                   .y = 0.5f*sinf(dt*0.0008 + (M_PI*2.0f / 3.0f)*1.0f) };
  *out++ = (Pos) { .x = 0.5f*cosf(dt*0.0008 + (M_PI*2.0f / 3.0f)*2.0f),
                   .y = 0.5f*sinf(dt*0.0008 + (M_PI*2.0f / 3.0f)*2.0f) };
  *out++ = (Pos) { .x = 0.5f*cosf(dt*0.0008 + (M_PI*2.0f / 3.0f)*3.0f),
                   .y = 0.5f*sinf(dt*0.0008 + (M_PI*2.0f / 3.0f)*3.0f) };

  /* more width: show more
     more height: zoom in */
  float aspect = (float)height / (float)width;
  for (Pos *p = state.vbuf; p != out; p++)
    p->x *= aspect;
}
