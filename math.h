#pragma once
#include "misc.h"
float round_tof(float x, float n);
int realmod(int x, int p);
float lerp(float a, float b, float t);
float lerp_rad(float a, float b, float t);
float lerp_round(float max, float a, float b, float t);
Vec2 add2(Vec2 a, Vec2 b);
Vec2 sub2(Vec2 a, Vec2 b);
Vec2 mul2_f(Vec2 a, float f);
Vec2 div2_f(Vec2 a, float f);
float dot2(Vec2 a, Vec2 b);
float mag2(Vec2 v);
Vec2 norm2(Vec2 v);
Vec3 add3(Vec3 a, Vec3 b);
Vec3 sub3(Vec3 a, Vec3 b);
Vec3 mul3_f(Vec3 a, float f);
Vec3 div3_f(Vec3 a, float f);
Vec3 lerp3(Vec3 a, Vec3 b, float t);
float dot3(Vec3 a, Vec3 b);
float mag3(Vec3 v);
Vec3 norm3(Vec3 v);
Vec3 cross3(Vec3 a, Vec3 b);
uint64_t fnv1_hash(void *key, int n_bytes);
Mat4 look_at4x4(Vec3 eye, Vec3 focus, Vec3 up);
Vec4 mul4x44(Mat4 m, Vec4 v);

float fmodf(float x, float n);
float signf(float x);
float fmaxf(float a, float b);
float fminf(float a, float b);

/* yoinked from WASM */
extern float cosf(float);
extern float sinf(float);
extern float randf(void);
extern float sqrtf(float);
extern float printff(float);
extern float atan2f(float, float);

Color color_lerp(Color c0, Color c1, float t);
