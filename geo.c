#include <stdint.h>

#include "gameplay.h"
#include "geo.h"

#include "math.h"

void geo_fireballs(Geo *geo) {
  for (int i = 0; i < ARR_LEN(state.fireballs); i++) {
    Fireball *f = state.fireballs + i;

    if (f->ts_fade_out > state.elapsed) {
      Vec2 pos = fireball_pos(f);
      float t = 1 - fireball_t(f);
      geo_8gon(geo, (Color){0.8f, 0.4f, 0.2f, 1.0f}, pos.y - 0.1f - t * 0.65f,
               pos.x, pos.y, 0.1f);
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
  Vec3 skin_color3 =
      lerp3((Vec3){0.20f, 0.25f, 0.43f}, (Vec3){0.26f, 0.19f, 0.43f},
            (float)id / (float)(UINT32_MAX));

  geo_man(geo, man,
          (Color){
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

  typedef struct {
    ManPartKind lhs, rhs;
  } PartPair;
  PartPair pp[] = {
      {ManPartKind_Neck, ManPartKind_Pelvis},
      {ManPartKind_Neck, ManPartKind_Elbow_R},
      {ManPartKind_Neck, ManPartKind_Elbow_L},
      {ManPartKind_Elbow_R, ManPartKind_Hand_R},
      {ManPartKind_Elbow_L, ManPartKind_Hand_L},
      {ManPartKind_Pelvis, ManPartKind_Knee_R},
      {ManPartKind_Pelvis, ManPartKind_Knee_L},
      {ManPartKind_Knee_R, ManPartKind_Sole_R},
      {ManPartKind_Knee_L, ManPartKind_Sole_L},
      {ManPartKind_Sole_R, ManPartKind_Toe_R},
      {ManPartKind_Sole_L, ManPartKind_Toe_L},
  };

  float thickness = 0.0125f;
  for (ManPartKind i = 0; i < ManPartKind_COUNT; i++) {
    Vec2 pos = man_pos(man, i);
    geo_8gon(geo, skin_color, z, pos.x, pos.y, thickness);
  }

  for (int i = 0; i < ARR_LEN(pp); i++)
    geo_line(geo, skin_color, z, man_pos(man, pp[i].lhs),
             man_pos(man, pp[i].rhs), thickness * 2.0f);
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

  geo_ibuf_push(geo, i + 0, i + 1, i + 2);
  geo_ibuf_push(geo, i + 2, i + 1, i + 3);
}

void geo_tri(Geo *geo, Vert a, Vert b, Vert c) {
  int i = geo->vbuf - geo->vbuf_base;

  geo_vbuf_push(geo, a);
  geo_vbuf_push(geo, b);
  geo_vbuf_push(geo, c);

  geo_ibuf_push(geo, i + 0, i + 1, i + 2);
}

void geo_ngon(Geo *geo, Color c, float z, float x, float y, float r, float n) {
  int i;
  int center = geo->vbuf - geo->vbuf_base;
  geo_vbuf_push(geo, (Vert){.pos = {x, y}, .z = z, .color = c});

#define CIRC_VERT(n)                                                           \
  ((Vert){.pos = {x + cosf(a * (n)) * r, y + sinf(a * (n)) * r},               \
          .z = z,                                                              \
          .color = c})

  float a = (M_PI * 2) / n;
  int start = geo->vbuf - geo->vbuf_base;
  geo_vbuf_push(geo, CIRC_VERT(1));

  for (int ni = 2; ni < (n + 1); ni++) {
    i = geo->vbuf - geo->vbuf_base;
    geo_vbuf_push(geo, CIRC_VERT(ni));
    geo_ibuf_push(geo, i, center, i - 1);
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
  if (tlen < 0.00000001f)
    return;
  float tx = nx / tlen;
  float ty = ny / tlen;

  geo_quad(geo, (Vert){.pos = {beg.x + tx, beg.y + ty}, .z = z, .color = c},
           (Vert){.pos = {beg.x - tx, beg.y - ty}, .z = z, .color = c},
           (Vert){.pos = {end.x + tx, end.y + ty}, .z = z, .color = c},
           (Vert){.pos = {end.x - tx, end.y - ty}, .z = z, .color = c});
}

void geo_rect(Geo *geo, Color c, float z, float x, float y, float w, float h) {
  geo_line(geo, c, z, (Vec2){.x = x - w / 2, .y = y},
           (Vec2){.x = x + w / 2, .y = y}, h);
}

void geo_text(Geo *geo, Color c, float z, float x, float y, char *str,
              float size) {
  if (str == 0 || !*str)
    return;

  do {
    geo_quad(
        geo,
        (Vert){.pos = {x + size, y - size},
               .z = z,
               .color = c,
               .letter = *str + 0.00f},
        (Vert){
            .pos = {x, y - size}, .z = z, .color = c, .letter = *str + 0.25f},
        (Vert){
            .pos = {x + size, y}, .z = z, .color = c, .letter = *str + 0.50f},
        (Vert){.pos = {x, y}, .z = z, .color = c, .letter = *str + 0.75f});

    x += size * state.letter_width_buf[(int)*str];

    z -= 0.00001f;
  } while (*++str);
}

void geo_tree(Geo *geo, float x, float _y, float size) {
  Vert *start = geo->vbuf;
  float w = 0.8f, h = GOLDEN_RATIO, r = 0.4f;
  float y = _y - 0.2f;

  geo_ngon(geo, COLOR_BARK, y, x, y + r, r, 32);
  geo_rect(geo, COLOR_BARK, y, x, h / 2 + y + r, w, h);

  geo_8gon(geo, COLOR_TREEGREEN, y - 1.8f, x + 0.80f, y + 2.2f, 0.8f);
  geo_8gon(geo, COLOR_TREEGREEN1, y - 1.8f, x + 0.16f, y + 3.0f, 1.0f);
  geo_8gon(geo, COLOR_TREEGREEN2, y - 1.8f, x - 0.80f, y + 2.5f, 0.9f);
  geo_8gon(geo, COLOR_TREEGREEN3, y - 1.8f, x - 0.16f, y + 2.0f, 0.8f);

  geo_8gon(geo, COLOR_TREEBORDER, y, x + 0.80f, y + 2.2f, 0.8f + 0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x + 0.16f, y + 3.0f, 1.0f + 0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x - 0.80f, y + 2.5f, 0.9f + 0.1f);
  geo_8gon(geo, COLOR_TREEBORDER, y, x - 0.16f, y + 2.0f, 0.8f + 0.1f);

  float sr = 1.22f;
  geo_8gon(geo, COLOR_FORESTSHADOW, y + sr, x, y + r, sr);

  for (Vert *v = start; v != geo->vbuf; v++)
    v->pos.x += (size - 1.0f) * (v->pos.x - x),
        v->pos.y += (size - 1.0f) * (v->pos.y - _y),
        v->z += (size - 1.0f) * (v->z - _y);
}

static void geo_geo(Geo *geo, Vec2 *vpos, uint16_t *ibuf, size_t ibuf_len,
                    float z, Vec2 pos, Color c, float scale) {
#define POS(o) mul2_f(vpos[ibuf[o]], scale)
  for (int j = 0; j < ibuf_len; j += 3) {
    geo_tri(geo,
            (Vert){.pos = add2(pos, POS(j + 0)), .z = pos.y + z, .color = c},
            (Vert){.pos = add2(pos, POS(j + 1)), .z = pos.y + z, .color = c},
            (Vert){.pos = add2(pos, POS(j + 2)), .z = pos.y + z, .color = c});
  }
#undef POS
}

void geo_mush(Geo *geo, float x, float y) {
  uint16_t mush_head_idx[] = {
      20, 15, 6,  0,  1,  2,  2,  3,  4,  4,  5,  16, 5,  6,  15, 6,  7,
      21, 7,  8,  21, 8,  22, 9,  9,  21, 8,  21, 10, 6,  10, 20, 6,  20,
      11, 15, 11, 12, 15, 12, 13, 15, 13, 23, 19, 23, 14, 19, 19, 15, 13,
      15, 16, 5,  16, 17, 4,  17, 18, 0,  0,  2,  4,  0,  4,  17,
  };
  Vec2 mush_head_vpos[] = {
      {-0.6832924485206604, 4.256103992462158},
      {-0.9851217269897461, 4.205738067626953},
      {-1.25424325466156, 4.060096263885498},
      {-1.4614933729171753, 3.834962844848633},
      {-1.5844132900238037, 3.5547335147857666},
      {-1.609682559967041, 3.2497758865356445},
      {-1.5728477239608765, 2.7070248126983643},
      {-1.8484810590744019, 2.4070963859558105},
      {-1.9843734502792358, 1.9949103593826294},
      {-1.7113581895828247, 1.3212904930114746},
      {-0.7709336876869202, 1.1408084630966187},
      {0.45371389389038086, 1.4698717594146729},
      {1.0233421325683594, 1.9367493391036987},
      {1.201648235321045, 2.449974775314331},
      {1.1400387287139893, 2.9653797149658203},
      {0.32721132040023804, 3.335967540740967},
      {0.09490837156772614, 3.834962844848633},
      {-0.11234179139137268, 4.060096740722656},
      {-0.38146305084228516, 4.205738067626953},
      {0.8156622648239136, 3.235445499420166},
      {-0.14220240712165833, 1.2397103309631348},
      {-1.2794300317764282, 1.1544814109802246},
      {-1.9463104009628296, 1.603408932685852},
      {1.2145967483520508, 2.7295539379119873},
  };

  uint16_t mush_stalk_idx[] = {
      6,  8, 3, 0,  1,  10, 1, 2, 10, 3, 4, 5, 2, 3, 9,
      10, 2, 9, 10, 11, 0,  8, 9, 3,  6, 7, 8, 3, 5, 6,
  };
  Vec2 mush_stalk_vpos[] = {
      {-0.5340940356254578, 2.079592704772949},
      {-0.6680429577827454, 1.8784172534942627},
      {-0.5222665667533875, 1.198749303817749},
      {-0.5029776692390442, 0.7197189331054688},
      {-0.7404330968856812, -0.05482161045074463},
      {-0.45051097869873047, -0.28512823581695557},
      {0.20138302445411682, -0.26937711238861084},
      {0.6960701942443848, -0.10995066165924072},
      {0.6608162522315979, 0.3705865144729614},
      {0.2774764895439148, 1.4442722797393799},
      {-0.02613319456577301, 2.0729384422302246},
      {-0.25050920248031616, 2.1934404373168945},
  };

  uint16_t mush_gills_idx[] = {
      17, 18, 11, 0,  1,  9,  1,  2,  8,  2,  3,  7,  3,  4,  6,  2,  7,  8,
      4,  5,  6,  6,  7,  3,  8,  9,  1,  9,  10, 0,  10, 11, 18, 11, 19, 17,
      10, 18, 0,  19, 12, 17, 12, 13, 16, 17, 12, 16, 13, 14, 15, 15, 16, 13,
  };
  Vec2 mush_gills_vpos[] = {
      {-0.49738210439682007, 2.574855089187622},
      {-0.8464001417160034, 2.421729564666748},
      {-1.163009524345398, 2.2250068187713623},
      {-1.4754316806793213, 1.9354982376098633},
      {-1.6194666624069214, 1.7306008338928223},
      {-1.6237162351608276, 1.5030369758605957},
      {-1.3879549503326416, 1.3286020755767822},
      {-1.1257306337356567, 1.2724579572677612},
      {-0.6789155602455139, 1.288960576057434},
      {-0.27490344643592834, 1.384753704071045},
      {0.018735185265541077, 1.4764454364776611},
      {0.39158689975738525, 1.6593329906463623},
      {0.9228411912918091, 2.1214256286621094},
      {1.038238763809204, 2.3853795528411865},
      {1.0553374290466309, 2.640036106109619},
      {0.9273452162742615, 2.785972833633423},
      {0.7028602361679077, 2.837226390838623},
      {0.3005169630050659, 2.7975387573242188},
      {-0.1379615068435669, 2.6836042404174805},
      {0.7146401405334473, 1.8876447677612305},
  };

  Vec2 pos = {x, y};
  float scale = 0.032f;
  Color c = COLOR_BEIGE;
  c.r -= 0.025f;
  c.g -= 0.05f;
  c.b -= 0.05f;
  geo_geo(geo, mush_gills_vpos, mush_gills_idx, ARR_LEN(mush_gills_idx),
          -0.17f * 0.2f, pos, c, scale);

  geo_geo(geo, mush_head_vpos, mush_head_idx, ARR_LEN(mush_head_idx),
          -0.16f * 0.2f, pos, COLOR_MAROON, scale);

  geo_geo(geo, mush_stalk_vpos, mush_stalk_idx, ARR_LEN(mush_stalk_idx),
          -0.18f * 0.2f, pos, COLOR_BEIGE, scale);
}

void geo_pot(Geo *geo, float x, float y) {
  uint16_t pot_inside_idx[] = {
      4,  3,  6,  2,  1,  8,  1,  0,  9,  9,  0,  10, 0,  19, 10, 19, 18, 11,
      10, 19, 11, 18, 17, 12, 17, 16, 13, 16, 15, 14, 14, 13, 16, 13, 12, 17,
      12, 11, 18, 9,  8,  1,  8,  7,  2,  7,  6,  3,  2,  7,  3,  6,  5,  4,
  };
  Vec2 pot_inside_vpos[] = {
      {0.0008260062895715237, 1.4859684705734253},
      {-0.2214391678571701, 1.4856634140014648},
      {-0.4222804009914398, 1.4599438905715942},
      {-0.6130747199058533, 1.4143027067184448},
      {-0.7220041155815125, 1.3386199474334717},
      {-0.7422056198120117, 1.2650517225265503},
      {-0.6866032481193542, 1.191085934638977},
      {-0.5840597152709961, 1.1304892301559448},
      {-0.42434412240982056, 1.0823992490768433},
      {-0.22309093177318573, 1.0515236854553223},
      {7.210289965087213e-08, 1.0408847332000732},
      {0.2226804494857788, 1.04638671875},
      {0.4389939606189728, 1.0708924531936646},
      {0.6241120100021362, 1.1117831468582153},
      {0.7378438115119934, 1.1818385124206543},
      {0.7641225457191467, 1.26528000831604},
      {0.6994263529777527, 1.3555537462234497},
      {0.5956566333770752, 1.4006905555725098},
      {0.4195736348628998, 1.4396936893463135},
      {0.22309091687202454, 1.4649922847747803},
  };

  uint16_t pot_legs_idx[] = {
      1, 2, 0, 4, 7, 6, 8, 11, 10, 1, 3, 2, 4, 5, 7, 8, 9, 11,
  };
  Vec2 pot_legs_vpos[] = {
      {-0.0711953341960907, -0.2052626609802246},
      {0.029802419245243073, -0.21043682098388672},
      {-0.09706591069698334, 0.252748966217041},
      {0.06084710359573364, 0.26309722661972046},
      {-0.4954726994037628, 0.0041942596435546875},
      {-0.46691256761550903, -0.05272102355957031},
      {-0.4023386836051941, 0.8988775014877319},
      {-0.39964908361434937, -0.02933502197265625},
      {0.34273383021354675, -0.01650214195251465},
      {0.37129396200180054, -0.07341742515563965},
      {0.3634302616119385, 0.8937033414840698},
      {0.4178609848022461, -0.03450906276702881},
  };

  uint16_t pot_lower_lip_idx[] = {
      15, 4,  5, 9,  10, 21, 21, 20, 9, 20, 19, 8,  9,  20, 8,
      19, 18, 7, 18, 17, 6,  17, 16, 5, 16, 15, 5,  15, 14, 4,
      14, 13, 3, 13, 12, 2,  12, 11, 1, 2,  12, 1,  11, 0,  1,
      14, 3,  4, 18, 6,  7,  13, 2,  3, 7,  8,  19, 5,  6,  17,
  };
  Vec2 pot_lower_lip_vpos[] = {
      {-0.8252643942832947, 1.243869423866272},
      {-0.7634760141372681, 1.1616746187210083},
      {-0.6495240926742554, 1.0943362712860107},
      {-0.47203943133354187, 1.0408960580825806},
      {-0.24839596450328827, 1.0065854787826538},
      {-0.000485121738165617, 0.9947627782821655},
      {0.24696941673755646, 1.0008769035339355},
      {0.487348735332489, 1.028109073638916},
      {0.6930619478225708, 1.0735490322113037},
      {0.8194469213485718, 1.1513983011245728},
      {0.848649263381958, 1.244123101234436},
      {-0.8252643942832947, 1.1507353782653809},
      {-0.7634760141372681, 1.0685405731201172},
      {-0.6495240926742554, 1.00120210647583},
      {-0.47203943133354187, 0.9477620124816895},
      {-0.24839596450328827, 0.9134514331817627},
      {-0.000485121738165617, 0.9016287326812744},
      {0.24696941673755646, 0.9077428579330444},
      {0.487348735332489, 0.9349750876426697},
      {0.6930619478225708, 0.9804149866104126},
      {0.8194469213485718, 1.0582642555236816},
      {0.848649263381958, 1.1509889364242554},
  };

  uint16_t pot_top_idx[] = {
      4,  3,  6,  2,  1,  8,  1,  0,  9,  9,  0,  10, 0,  19, 10, 19, 18, 11,
      10, 19, 11, 18, 17, 12, 17, 16, 13, 16, 15, 14, 14, 13, 16, 13, 12, 17,
      12, 11, 18, 9,  8,  1,  8,  7,  2,  7,  6,  3,  2,  7,  3,  6,  5,  4,
  };
  Vec2 pot_top_vpos[] = {
      {0.00043270131573081017, 1.4893641471862793},
      {-0.24656043946743011, 1.4890251159667969},
      {-0.46974611282348633, 1.4604442119598389},
      {-0.6817671656608582, 1.4097251892089844},
      {-0.8028153777122498, 1.3256224393844604},
      {-0.8252643942832947, 1.243869423866272},
      {-0.7634760141372681, 1.1616746187210083},
      {-0.6495240926742554, 1.0943362712860107},
      {-0.47203943133354187, 1.0408960580825806},
      {-0.24839596450328827, 1.0065854787826538},
      {-0.000485121738165617, 0.9947627782821655},
      {0.24696941673755646, 1.0008769035339355},
      {0.487348735332489, 1.028109073638916},
      {0.6930619478225708, 1.0735490322113037},
      {0.8194469213485718, 1.1513983011245728},
      {0.848649263381958, 1.244123101234436},
      {0.7767553329467773, 1.3444401025772095},
      {0.6614407896995544, 1.3945986032485962},
      {0.46576783061027527, 1.437941074371338},
      {0.24742554128170013, 1.4660542011260986},
  };

  uint16_t pot_body_idx[] = {
      9, 0, 5, 0, 1, 4, 1, 2, 3, 4, 1, 3,  4, 5,
      0, 5, 6, 8, 9, 5, 8, 6, 7, 8, 9, 10, 0,
  };
  Vec2 pot_body_vpos[] = {
      {-0.8051009178161621, 1.123082160949707},
      {-0.7242040038108826, 0.6156213879585266},
      {-0.6049063801765442, 0.43390172719955444},
      {-0.42062073945999146, 0.2910988926887512},
      {-0.2023766189813614, 0.21215593814849854},
      {0.004688992630690336, 0.19517719745635986},
      {0.34291207790374756, 0.24794721603393555},
      {0.5758152604103088, 0.3799751400947571},
      {0.7026930451393127, 0.6055106520652771},
      {0.7594919800758362, 0.8143225908279419},
      {0.797349750995636, 1.0511738061904907},
  };
  /* body, inside, legs, lower_lip, top */

  Vec2 pos = {x, y};
  float scale = 0.28f;
  geo_geo(geo, pot_inside_vpos, pot_inside_idx, ARR_LEN(pot_inside_idx),
          -0.0029f, pos, (Color){0.25f, 0.25f, 0.25f + 0.1f, 1.00f}, scale);

  geo_geo(geo, pot_body_vpos, pot_body_idx, ARR_LEN(pot_body_idx), -0.0027f,
          pos, (Color){0.31f, 0.31f, 0.31f + 0.1f, 1.00f}, scale);

  geo_geo(geo, pot_legs_vpos, pot_legs_idx, ARR_LEN(pot_legs_idx), -0.0024f,
          pos, (Color){0.27f, 0.27f, 0.27f + 0.1f, 1.00f}, scale);

  geo_geo(geo, pot_lower_lip_vpos, pot_lower_lip_idx,
          ARR_LEN(pot_lower_lip_idx), -0.0029f, pos,
          (Color){0.28f, 0.28f, 0.28f + 0.1f, 1.00f}, scale);

  geo_geo(geo, pot_top_vpos, pot_top_idx, ARR_LEN(pot_top_idx), -0.0028f, pos,
          (Color){0.34f, 0.34f, 0.34f + 0.1f, 1.00f}, scale);
}
