#include "math.h"

static float round_tof(float x, float n) { return n * (int)(x/n); }
static int realmod(int x, int p) { return ((x % p) + p) % p; }

static float lerp(float a, float b, float t) { return (1.0f-t)*a+t*b; }
/* dont tell mom its just lerp_round with tau as max */
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

// static Vec2 vec2_pivot(Vec2 o, Vec2 v, float rads) {
//   float oldX = v.x - o.x;
//   float oldY = v.y - o.y;
//   return (Vec2) {
//     .x = o.x + oldX * cosf(rads) - oldY * sinf(rads),
//     .y = o.y + oldX * sinf(rads) + oldY * cosf(rads)
//   };
// }
static Vec2 add2(Vec2 a, Vec2 b) { return (Vec2) { a.x + b.x,
                                                   a.y + b.y, }; }
static Vec2 sub2(Vec2 a, Vec2 b) { return (Vec2) { a.x - b.x,
                                                   a.y - b.y, }; }
static Vec2 mul2_f(Vec2 a, float f) { return (Vec2) { a.x * f,
                                                      a.y * f, }; }
static Vec2 div2_f(Vec2 a, float f) { return (Vec2) { a.x / f,
                                                      a.y / f, }; }
static float dot2(Vec2 a, Vec2 b) { return a.x*b.x + a.y*b.y; }
static float mag2(Vec2 v) { return sqrtf(dot2(v, v)); }
static Vec2 norm2(Vec2 v) { return div2_f(v, mag2(v)); }

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
/* beloved fnv */
uint64_t fnv1_hash(void *key, int n_bytes) {
    unsigned char *p = key;
    uint64_t h = 14695981039346656037ul;
    for (int i = 0; i < n_bytes; i++)
        h = (h * 1099511628211) ^ p[i];
    return h;
}

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