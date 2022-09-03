#include "math.h"

/* the blob of math fns llvm tries to include for us add 20kb to the WASM smh */
float round_tof(float x, float n) { return n * (int)(x / n); }
int realmod(int x, int p) { return ((x % p) + p) % p; }

float fmodf(float x, float n) { return x - n * (int)(x / n); }
float signf(float x) { return (x < 0.0f) ? -1.0f : 1.0f; }
float fmaxf(float a, float b) { return (a > b) ? a : b; }
float fminf(float a, float b) { return (a < b) ? a : b; }
float norm_clampf(float t) {
  t = fmaxf(t, 0.0f);
  t = fminf(t, 1.0f);
  return t;
}

float lerp(float a, float b, float t) { return (1.0f - t) * a + t * b; }
/* don't tell mom its just lerp_round with tau as max */
float lerp_rad(float a, float b, float t) {
  float difference = fmodf(b - a, M_PI * 2.0f),
        distance = fmodf(2.0f * difference, M_PI * 2.0f) - difference;
  return a + distance * t;
}
float lerp_round(float max, float a, float b, float t) {
  float difference = fmodf(b - a, max),
        distance = fmodf(2.0f * difference, max) - difference;
  return a + distance * t;
}

//  Vec2 vec2_pivot(Vec2 o, Vec2 v, float rads) {
//   float oldX = v.x - o.x;
//   float oldY = v.y - o.y;
//   return (Vec2) {
//     .x = o.x + oldX * cosf(rads) - oldY * sinf(rads),
//     .y = o.y + oldX * sinf(rads) + oldY * cosf(rads)
//   };
// }
Vec2 add2(Vec2 a, Vec2 b) {
  return (Vec2){
      a.x + b.x,
      a.y + b.y,
  };
}
Vec2 sub2(Vec2 a, Vec2 b) {
  return (Vec2){
      a.x - b.x,
      a.y - b.y,
  };
}
Vec2 mul2_f(Vec2 a, float f) {
  return (Vec2){
      a.x * f,
      a.y * f,
  };
}
Vec2 div2_f(Vec2 a, float f) {
  return (Vec2){
      a.x / f,
      a.y / f,
  };
}
Vec2 perp2(Vec2 n) { return (Vec2) { -n.y, n.x }; }
float dot2(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
float mag2(Vec2 v) { return sqrtf(dot2(v, v)); }
Vec2 norm2(Vec2 v) { return div2_f(v, mag2(v)); }

Vec3 add3(Vec3 a, Vec3 b) {
  return (Vec3){
      a.x + b.x,
      a.y + b.y,
      a.z + b.z,
  };
}
Vec3 sub3(Vec3 a, Vec3 b) {
  return (Vec3){
      a.x - b.x,
      a.y - b.y,
      a.z - b.z,
  };
}
Vec3 mul3_f(Vec3 a, float f) {
  return (Vec3){
      a.x * f,
      a.y * f,
      a.z * f,
  };
}
Vec3 div3_f(Vec3 a, float f) {
  return (Vec3){
      a.x / f,
      a.y / f,
      a.z / f,
  };
}
Vec3 lerp3(Vec3 a, Vec3 b, float t) {
  return add3(mul3_f(a, 1.0f - t), mul3_f(b, t));
}
float dot3(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float mag3(Vec3 v) { return sqrtf(dot3(v, v)); }
Vec3 norm3(Vec3 v) { return div3_f(v, mag3(v)); }
Vec3 cross3(Vec3 a, Vec3 b) {
  return (Vec3){(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z),
                (a.x * b.y) - (a.y * b.x)};
}
/* beloved fnv */
uint64_t fnv1_hash(void *key, int n_bytes) {
  unsigned char *p = key;
  uint64_t h = 14695981039346656037ul;
  for (int i = 0; i < n_bytes; i++)
    h = (h * 1099511628211) ^ p[i];
  return h;
}

Mat4 look_at4x4(Vec3 eye, Vec3 focus, Vec3 up) {
  Vec3 eye_dir = sub3(focus, eye);
  Vec3 R2 = norm3(eye_dir);

  Vec3 R0 = norm3(cross3(up, R2));
  Vec3 R1 = cross3(R2, R0);

  Vec3 neg_eye = mul3_f(eye, -1.0f);

  float D0 = dot3(R0, neg_eye);
  float D1 = dot3(R1, neg_eye);
  float D2 = dot3(R2, neg_eye);

  return (Mat4){{{R0.x, R1.x, R2.x, 0.0f},
                 {R0.y, R1.y, R2.y, 0.0f},
                 {R0.z, R1.z, R2.z, 0.0f},
                 {D0, D1, D2, 1.0f}}};
}

Vec4 mul4x44(Mat4 m, Vec4 v) {
  Vec4 res;
  for (int x = 0; x < 4; ++x) {
    float sum = 0;
    for (int y = 0; y < 4; ++y)
      sum += m.nums[y][x] * v.nums[y];

    res.nums[x] = sum;
  }
  return res;
}

Color color_lerp(Color c0, Color c1, float t) {
  return (Color){
      .r = lerp(c0.r, c1.r, t),
      .g = lerp(c0.g, c1.g, t),
      .b = lerp(c0.b, c1.b, t),
      .a = lerp(c0.a, c1.a, t),
  };
}
