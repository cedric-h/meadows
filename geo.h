#pragma once
#include "misc.h"
/* a collection of geometry (vertices, indices) you can write into */
typedef struct {
    Vert *vbuf;
    Vert *vbuf_base;
    size_t vbuf_max;

    uint16_t *ibuf;
    uint16_t *ibuf_base;
    size_t ibuf_max;
} Geo;

void geo_fireballs(Geo *geo);
void geo_labels(Geo *geo);
void geo_man_id(Geo *geo, Man *man, uint32_t id);
void geo_man(Geo *geo, Man *man, Color skin_color);
void geo_ibuf_push(Geo *geo, uint16_t a, uint16_t b, uint16_t c);
void geo_vbuf_push(Geo *geo, Vert v);
void geo_quad(Geo *geo, Vert tl, Vert tr, Vert br, Vert bl);
void geo_tri(Geo *geo, Vert a, Vert b, Vert c);
void geo_ngon(Geo *geo, Color c, float z, float x, float y, float r, float n);
void geo_8gon(Geo *geo, Color c, float z, float x, float y, float r);
void geo_line(Geo *geo, Color c, float z, Vec2 beg, Vec2 end, float thickness);
void geo_rect(Geo *geo, Color c, float z, float x, float y, float w, float h);
void geo_text(Geo *geo, Color c, float z, float x, float y, char *str, float size);
void geo_tree(Geo *geo, float x, float _y, float size);
void geo_pot(Geo *geo, float x, float y);
void geo_mush(Geo *geo, float x, float y);

