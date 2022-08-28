#pragma once
#include "misc.h"
static float round_tof(float x, float n);
static int realmod(int x, int p);
static float lerp(float a, float b, float t);
static float lerp_rad(float a, float b, float t);
static float lerp_round(float max, float a, float b, float t);
static Vec2 add2(Vec2 a, Vec2 b);
static Vec2 sub2(Vec2 a, Vec2 b);
static Vec2 mul2_f(Vec2 a, float f);
static Vec2 div2_f(Vec2 a, float f);
static float dot2(Vec2 a, Vec2 b);
static float mag2(Vec2 v);
static Vec2 norm2(Vec2 v);
static Vec3 add3(Vec3 a, Vec3 b);
static Vec3 sub3(Vec3 a, Vec3 b) ;
static Vec3 mul3_f(Vec3 a, float f) ;
static Vec3 div3_f(Vec3 a, float f);
static Vec3 lerp3(Vec3 a, Vec3 b, float t);
static float dot3(Vec3 a, Vec3 b);
static float mag3(Vec3 v);
static Vec3 norm3(Vec3 v) ;
static Vec3 cross3(Vec3 a, Vec3 b);
uint64_t fnv1_hash(void *key, int n_bytes) ;
static Mat4 look_at4x4(Vec3 eye, Vec3 focus, Vec3 up) ;
static Vec4 mul4x44(Mat4 m, Vec4 v);

