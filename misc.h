#pragma once
#include <stdint.h>
#include <stddef.h>
#include "stb_perlin.h"

#define M_PI 3.141592653589793
#define GOLDEN_RATIO (1.618034f)
#define ARR_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define abs(n) (((n) < 0) ? -(n) : (n))

#define MAN_FRAME_COUNT (5)

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

#define STB_PERLIN_IMPLEMENTATION

#define LABEL_TEXT_SIZE (0.10f)
#include "stb_perlin.h"
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y; } Vec2;
typedef union {
    Vec3 xyz;
    struct { float x, y, z, w; };
    float nums[4];
} Vec4;
typedef struct { float nums[4][4]; } Mat4;
typedef struct { float r, g, b, a; } Color;

typedef struct { Vec2 pos; float z, letter; Color color; } Vert;

/* an animated dude! */
typedef struct {
    Vec2 pos;
    float anim_prog, anim_damp;
    float dir;
} Man;


typedef enum {
    ManPartKind_Elbow_L,
    ManPartKind_Elbow_R,
    ManPartKind_Hand_L,
    ManPartKind_Hand_R,
    ManPartKind_Head,
    ManPartKind_Knee_L,
    ManPartKind_Knee_R,
    ManPartKind_Neck,
    ManPartKind_Pelvis,
    ManPartKind_Sole_L,
    ManPartKind_Sole_R,
    ManPartKind_Toe_L,
    ManPartKind_Toe_R,
    ManPartKind_COUNT,
} ManPartKind;
typedef struct {
    float time;
    Vec3 pos[ManPartKind_COUNT];
} ManPartFrame;
typedef struct {
    ManPartFrame frames[MAN_FRAME_COUNT*2 - 2];
} ManFrames;


/* another player (over the network!) */
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

/* a persistent (enough) bit of text in world-space
 * rendered at the end of the frame (because transparency) */
typedef struct {
    char msg[25];
    double ts_pop_in, ts_fade_out;
    float x, y, drift;

    /* TODO: fade out labels before removing them */
} Label;

/* i got an inkling this code generalizes to more than just mushrooms */
typedef enum {
    MushroomStage_Undiscovered,
    MushroomStage_Ripe,
    MushroomStage_Collected,
    MushroomStage_Blasted,
} MushroomStage;
typedef struct {
    MushroomStage stage;
    Vec2 pos;
    uint32_t hash;
} Mushroom;

typedef struct {
    Vec2 start, target;
    double ts_fade_out, ts_spawned;
} Fireball;

typedef enum {
    PlayerAction_Walking,
    PlayerAction_Casting,
} PlayerAction;

static struct {
    double elapsed;

    /* graphics */
    Vert vbuf[1 << 16];
    uint16_t ibuf[1 << 17];
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
    struct {
        Vec2 vel;
        Man man;
        PlayerAction action;

        Vec2 cast_target;
    } player;

    /* ui */
    float letter_width_buf[128];
    char todo[5][25];
    Label labels[64];

    /* gameplay? */
    Mushroom mushrooms[1 << 8];
    Fireball fireballs[1 << 6];

} state = {0};

#define WASM_EXPORT __attribute__((visibility("default")))

/* the blob of magh fns llvm tries to include for us add 20kb to the WASM smh */
static float fmodf(float x, float n) { return x - n * (int)(x/n); }
static float signf(float x) { return (x < 0.0f) ? -1.0f : 1.0f; }
static float fmaxf(float a, float b) { return (a > b) ? a : b; }
static float fminf(float a, float b) { return (a < b) ? a : b; }
extern float cosf(float);
extern float sinf(float);
extern float randf(void);
extern float sqrtf(float);
extern float printff(float);
extern float atan2f(float, float);

/* shared memory with our JS host */
extern void   vbuf(void *ptr, int len);
extern void   ibuf(void *ptr, int len);
extern void netbuf(void *ptr, int len);
