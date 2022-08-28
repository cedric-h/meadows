#include <stdint.h>

#include "geo.h"
#include "gameplay.h"

#include "math.h"

 void geo_fireballs(Geo *geo) {
    for (int i = 0; i < ARR_LEN(state.fireballs); i++) {
        Fireball *f = state.fireballs + i;

        if (f->ts_fade_out > state.elapsed) {
            Vec2 pos = fireball_pos(f);
            float t = 1 - fireball_t(f);
            geo_8gon(geo, (Color) { 0.8f, 0.4f, 0.2f, 1.0f }, pos.y-0.1f-t*0.65f, pos.x, pos.y, 0.1f);
        }
    }
}

 void geo_labels(Geo *geo) {
    for (int i = 0; i < ARR_LEN(state.labels); i++) {
        Label *l = state.labels + i;

        if (l->ts_fade_out > state.elapsed && l->ts_pop_in < state.elapsed) {
            float duration = l->ts_fade_out - l->ts_pop_in;
            float y = l->y + l->drift * ((state.elapsed - l->ts_pop_in) / duration);
            geo_text(geo, COLOR_WHITE, -0.99f, l->x, y, l->msg, LABEL_TEXT_SIZE);
        }
    }
}

 void geo_man_id(Geo *geo, Man *man, uint32_t id) {
    Vec3 skin_color3 = lerp3(
            (Vec3) { 0.20f, 0.25f, 0.43f },
            (Vec3) { 0.26f, 0.19f, 0.43f },
            (float)id/(float)(UINT32_MAX));

    geo_man(geo, man, (Color) {
            .r = skin_color3.x,
            .g = skin_color3.y,
            .b = skin_color3.z,
            .a = 1.0f,
    });
}

 void geo_man(Geo *geo, Man *man, Color skin_color) {
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

 void geo_ibuf_push(Geo *geo, uint16_t a, uint16_t b, uint16_t c) {
    if ((geo->ibuf - geo->ibuf_base) < geo->ibuf_max)
        *geo->ibuf++ = a, *geo->ibuf++ = b, *geo->ibuf++ = c;
    else
        printff(500);
}

 void geo_vbuf_push(Geo *geo, Vert v) {
    if ((geo->vbuf - geo->vbuf_base) < geo->vbuf_max)
        *geo->vbuf++ = v;
    else
        printff(500);
}

 void geo_quad(Geo *geo, Vert tl, Vert tr, Vert br, Vert bl) {
    int i = geo->vbuf - geo->vbuf_base;

    geo_vbuf_push(geo, tl);
    geo_vbuf_push(geo, tr);
    geo_vbuf_push(geo, br);
    geo_vbuf_push(geo, bl);

    geo_ibuf_push(geo, i+0, i+1, i+2);
    geo_ibuf_push(geo, i+2, i+1, i+3);
}

 void geo_tri(Geo *geo, Vert a, Vert b, Vert c) {
    int i = geo->vbuf - geo->vbuf_base;

    geo_vbuf_push(geo, a);
    geo_vbuf_push(geo, b);
    geo_vbuf_push(geo, c);

    geo_ibuf_push(geo, i+0, i+1, i+2);
}

 void geo_ngon(Geo *geo, Color c, float z, float x, float y, float r, float n) {
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
 void geo_8gon(Geo *geo, Color c, float z, float x, float y, float r) {
    geo_ngon(geo, c, z, x, y, r, 8.0f);
}

 void geo_line(Geo *geo, Color c, float z, Vec2 beg, Vec2 end, float thickness) {
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

 void geo_rect(Geo *geo, Color c, float z, float x, float y, float w, float h) {
    geo_line(geo, c, z, (Vec2) { .x = x-w/2, .y = y},
             (Vec2) { .x = x+w/2, .y = y}, h);
}

 void geo_text(Geo *geo, Color c, float z, float x, float y, char *str, float size) {
    if (str == 0 || !*str) return;

    do {
        geo_quad(geo,
                 (Vert) { .pos = { x + size, y - size }, .z=z, .color=c, .letter=*str + 0.00f },
                 (Vert) { .pos = { x       , y - size }, .z=z, .color=c, .letter=*str + 0.25f },
                 (Vert) { .pos = { x + size, y        }, .z=z, .color=c, .letter=*str + 0.50f },
                 (Vert) { .pos = { x       , y        }, .z=z, .color=c, .letter=*str + 0.75f }
        );

        x += size * state.letter_width_buf[(int)*str];

        z -= 0.00001f;
    } while(*++str);
}

 void geo_tree(Geo *geo, float x, float _y, float size) {
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