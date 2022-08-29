#include "gameplay.h"
#include "math.h"

State state = {0};

 float fireball_t(Fireball *f) {
    float duration = f->ts_fade_out - f->ts_spawned;
    return (state.elapsed - f->ts_spawned) / duration;
}

 Vec2 fireball_pos(Fireball *f) {
    float t = fireball_t(f);
    return (Vec2) {
            .x = lerp(f->start.x, f->target.x, t),
            .y = lerp(f->start.y, f->target.y, t),
    };
}

static float text_width(char *str, float size) {
    /* I could simplify this loop and then verify it does the same thing as the one in
     * geo_text ... OR I could just keep this trimmed down version of the one there ... */

    if (str == 0 || !*str) return 0.0f;

    float ret = 0.0f;
    do {
        ret += size * state.letter_width_buf[(int)*str];
    } while(*++str);

    return ret;
}

static void labels_push(Label new) {
    new.x -= text_width(new.msg, LABEL_TEXT_SIZE)*0.5f;

    for (int i = 0; i < ARR_LEN(state.labels); i++) {
        Label *l = state.labels + i;
        if (l->ts_fade_out <= state.elapsed) {
            *l = new;
            return;
        }
    }
}

#define SHOW_TODO(i, s) __builtin_memcpy(state.todo[(i)], (s), sizeof(s))
 void quest(Geo *geo, Mushroom **onscreen_mush, float dt) {

    /* no matter what, these exist in the world */
    Vec2 wiz = { 1.5f, 1.2f };

    geo_man(geo,
            &(Man){ .dir = -0.88f, .pos = wiz },
            (Color) { 0.19f, 0.24f, 0.60f, 1.0f }
    );

    float pot_x = wiz.x - 0.325f;
    float pot_y = wiz.y + 0.35f;
    // geo_8gon(geo, COLOR_DARKGREY, pot_y-0.25f, pot_x, pot_y, 0.25f);
    geo_pot(geo, pot_x, pot_y);

    /* rest of this file is stuff that changes based on where you are in quest */
    typedef enum {
        QuestStage_Exclamation, /* waiting on player to start it */
        QuestStage_MushPickin,
        QuestStage_MushPickinDone,
        QuestStage_MushRoastin,
        QuestStage_MushRoastinDone,
    } QuestStage;

    static QuestStage stage = QuestStage_Exclamation;
    static int mushies = 0;

    uint8_t near_wiz = mag2(sub2(wiz, state.player.man.pos)) < 0.8f;

    switch (stage) {

        case QuestStage_Exclamation: {
            SHOW_TODO(0, "- talk to wizard");

            if (near_wiz) {
                SHOW_TODO(1, "- (press E)");

                if (state.keys_down['e']) {
                    stage = QuestStage_MushPickin;

                    Label msgs[] = {
                            { .msg = "don't just stand there" },
                            { .msg =  "get me some shrooms!" },
                    };

                    for (int i = 0; i < ARR_LEN(msgs); i++)
                        label_push_i(msgs + i, i, wiz);
                }
            }

            float excl_y = wiz.y+0.9f + sinf(state.elapsed * 0.06f) * 0.06f;
            geo_rect(geo, COLOR_TEXT, excl_y-1.0f, wiz.x, excl_y, 0.04f, 0.2f);

        } break;

        case QuestStage_MushPickin: {

            SHOW_TODO(0, "- find mushrooms");
            switch (mushies) {
                case 0: SHOW_TODO(1, "  [0/3] collected"); break;
                case 1: SHOW_TODO(1, "  two more to go!");     break;
                case 2: SHOW_TODO(1, "  LAST ONE!");     break;
            }

            for (int i = 0; onscreen_mush[i]; i++) {
                Mushroom *m = onscreen_mush[i];

                if (m->stage == MushroomStage_Ripe &&
                    mag2(sub2(m->pos, state.player.man.pos)) < 0.4f)
                {
                    SHOW_TODO(2, "- (press E)");

                    if (state.keys_down['e']) {
                        m->stage = MushroomStage_Collected;
                        mushies++;
                        if (mushies > 2) stage = QuestStage_MushPickinDone;
                    }
                }
            }
        } break;

        case QuestStage_MushPickinDone: {
            SHOW_TODO(0, "report back to wiz");

            if (near_wiz) {
                SHOW_TODO(1, "- (press E)");

                if (state.keys_down['e']) {
                    mushies = 0;
                    stage = QuestStage_MushRoastin;

                    Label msgs[] = {
                            { .msg =  "oh, this'll never do!"  },
                            { .msg = "you gotta roast 'em ..." },
                            { .msg =  "here, take this spoon"  },
                    };

                    for (int i = 0; i < ARR_LEN(msgs); i++)
                        label_push_i(msgs + i, i, wiz);
                }
            }
        } break;

        case QuestStage_MushRoastin: {
            SHOW_TODO(0, "- roast 3 mushies");

            switch (mushies) {
                case 0: {
                    SHOW_TODO(1, "  (hold space)");
                    SHOW_TODO(2, "  (aim with WASD)");
                    SHOW_TODO(3, "  (release space)");
                    SHOW_TODO(4, "  [0/3] roasted");
                } break;
                case 1: SHOW_TODO(1, "  two more to cook!");     break;
                case 2: SHOW_TODO(1, "  LAST ONE!");     break;
            }

            /* mushroom v. fireball collision */
            for (int i = 0; onscreen_mush[i]; i++) {
                Mushroom *m = onscreen_mush[i];

                for (int i = 0; i < ARR_LEN(state.fireballs); i++) {
                    Fireball *f = state.fireballs + i;

                    if (f->ts_fade_out > state.elapsed) {
                        Vec2 fpos = fireball_pos(f);

                        // use t to stop fireballs that are still "in the air" from hitting mushies
                        // float t = fireball_t(f);
                        // if (t > 0.8f && mag2(sub2(fpos, m->pos)) < 0.2f)

                        if (mag2(sub2(fpos, m->pos)) < 0.2f)
                            if (m->stage == MushroomStage_Ripe ||
                                m->stage == MushroomStage_Collected) {
                                m->stage = MushroomStage_Blasted;
                                mushies++;
                                if (mushies > 2) stage = QuestStage_MushRoastinDone;
                            }
                    }
                }
            }
        } break;

        case QuestStage_MushRoastinDone: {
            SHOW_TODO(0, "good job!");
        } break;
    }
    /* TODO:
        - lerp dir to face player
        - gradually type in TODO letters?!
    */

}
#undef SHOW_TODO

 void man_anim(Man *man, float dt, Vec2 vel) {
    uint8_t going = dot2(vel, vel) > 0.00001f;
    man->anim_damp = lerp(man->anim_damp, going, dt * 0.15f);
    man->anim_prog += 0.14f*man->anim_damp*dt;
    int anim_len = ARR_LEN(state.mf.frames);
    man->anim_prog = lerp_round(anim_len,
                                man->anim_prog, going*man->anim_prog, dt*0.05f);
    man->anim_prog = fmodf(man->anim_prog, anim_len);

    man->dir = lerp_rad(man->dir, atan2f(-vel.y, -vel.x), going*dt*0.1f);
}

 Vec2 man_pos(Man *man, ManPartKind mpk) {
    float q = man->anim_prog;
    float t = q - round_tof(q, 1.0f);

    int len = ARR_LEN(state.mf.frames);
    int i = (int)(q+0  ) % len;
    int n = (int)(q+0+1) % len;

    Vec3 a = state.mf.frames[i].pos[mpk];
    Vec3 b = state.mf.frames[n].pos[mpk];
    Vec4 p = { .x = lerp(a.x, b.x, t) * 1.35f,
            .y = lerp(a.y, b.y, t),
            .z = lerp(a.z, b.z, t),
            .w = 1.0f };

    float mx = cosf(man->dir);
    float my = sinf(man->dir);
    Mat4 m = look_at4x4((Vec3) { mx, my, 0.75f },
                        (Vec3) {  0,  0, 0     },
                        (Vec3) {  0,  0, 1     });
    p = mul4x44(m, p);
    return (Vec2) { p.x * 0.5f + man->pos.x,
                    p.y * 0.5f + man->pos.y };
}

 void label_push_i(Label *l, int i, Vec2 p) {
    l->drift = (LABEL_TEXT_SIZE * 1.1f) * 3;

    l->x = p.x;
    l->y = p.y + 0.75f;

    float delay = i * 100.0f;
    l->ts_pop_in = state.elapsed + delay;
    l->ts_fade_out = state.elapsed + 300.0f + delay;
    labels_push(*l);
}

void fireballs_push(Fireball new) {
    for (int i = 0; i < ARR_LEN(state.fireballs); i++) {
        Fireball *f = state.fireballs + i;
        if (f->ts_fade_out <= state.elapsed) {
            *f = new;
            return;
        }
    }
}
