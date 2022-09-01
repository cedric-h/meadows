#include "gameplay.h"
#include "math.h"

State state = {0};

/* how far between "spawned" and "charged" are you? */
float fireball_charging_t(Fireball *f) {
  float duration = f->ts_charged - f->ts_spawned;
  return (state.elapsed - f->ts_spawned) / duration;
}

/* how far between "charged" and "exploded" are you? */
float fireball_flying_t(Fireball *f) {
  float duration = f->ts_exploded - f->ts_charged;
  return (state.elapsed - f->ts_charged) / duration;
}

/* how far between "charged" and "exploded" are you? */
float fireball_dying_t(Fireball *f) {
  float duration = f->ts_fade_out - f->ts_exploded;
  return (state.elapsed - f->ts_exploded) / duration;
}

/* the time between when the fireball appears and when it dies; its whole
 * lifetime */
float fireball_t(Fireball *f) {
  float duration = f->ts_fade_out - f->ts_spawned;
  return (state.elapsed - f->ts_spawned) / duration;
}

Vec2 fireball_pos(Fireball *f) {
  float t = norm_clampf(fireball_flying_t(f));
  return (Vec2){
      .x = lerp(f->start.x, f->target.x, t),
      .y = lerp(f->start.y, f->target.y, t),
  };
}

static float text_width(char *str, float size) {
  /* I could simplify this loop and then verify it does the same thing as the
   * one in geo_text ... OR I could just keep this trimmed down version of the
   * one there ... */

  if (str == 0 || !*str)
    return 0.0f;

  float ret = 0.0f;
  do {
    ret += size * state.letter_width_buf[(int)*str];
  } while (*++str);

  return ret;
}

static void labels_push(Label new) {
  new.x -= text_width(new.msg, LABEL_TEXT_SIZE) * 0.5f;

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
  const float wiz_x = 1.5f;
  const float wiz_y = 1.2f;
  Vec2 wiz = {wiz_x, wiz_y};

  geo_man(geo, &(Man){.dir = -0.88f, .pos = wiz, .hp = 1.0f},
          (Color){0.19f, 0.24f, 0.60f, 1.0f});

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
    QuestStage_SaveWizFromMushMen,
    QuestStage_Done,
  } QuestStage;

  static QuestStage stage = QuestStage_Exclamation;
  // static QuestStage stage = QuestStage_SaveWizFromMushMen;

  const float talking_dist = 0.8f;
  const float seeing_dist = 2.7f;
  float wiz_dist = mag2(sub2(wiz, state.player.man.pos));

  switch (stage) {

  case QuestStage_Exclamation: {
    SHOW_TODO(0, "- talk to wizard");

    if (wiz_dist < talking_dist) {
      SHOW_TODO(1, "- (press E)");

      if (state.keys_down['e']) {
        stage = QuestStage_MushPickin;

        Label msgs[] = {
            {.msg = "don't just stand there"},
            {.msg = "get me some shrooms!"},
        };

        for (int i = 0; i < ARR_LEN(msgs); i++)
          label_push_i(msgs + i, i, wiz);
      }
    }

    float excl_y = wiz.y + 0.9f + sinf(state.elapsed * 3.6f) * 0.06f;
    geo_rect(geo, COLOR_TEXT, excl_y - 1.0f, wiz.x, excl_y, 0.04f, 0.2f);

  } break;

  case QuestStage_MushPickin: {
    static int mushies = 0;

    SHOW_TODO(0, "- find mushrooms");
    switch (mushies) {
    case 0:
      SHOW_TODO(1, "  [0/3] collected");
      break;
    case 1:
      SHOW_TODO(1, "  two more to go!");
      break;
    case 2:
      SHOW_TODO(1, "  LAST ONE!");
      break;
    }

    for (int i = 0; onscreen_mush[i]; i++) {
      Mushroom *m = onscreen_mush[i];

      if (m->stage == MushroomStage_Ripe &&
          mag2(sub2(m->pos, state.player.man.pos)) < 0.4f) {
        SHOW_TODO(2, "- (press E)");

        if (state.keys_down['e']) {
          m->stage = MushroomStage_Collected;
          mushies++;
          if (mushies > 2)
            stage = QuestStage_MushPickinDone;
        }
      }
    }
  } break;

  case QuestStage_MushPickinDone: {
    SHOW_TODO(0, "report back to wiz");

    if (wiz_dist < talking_dist) {
      SHOW_TODO(1, "- (press E)");

      if (state.keys_down['e']) {
        stage = QuestStage_MushRoastin;

        Label msgs[] = {
            {.msg = "oh, this'll never do!"},
            {.msg = "you gotta roast 'em ..."},
            {.msg = "here, take this spoon"},
        };

        for (int i = 0; i < ARR_LEN(msgs); i++)
          label_push_i(msgs + i, i, wiz);
      }
    }
  } break;

  case QuestStage_MushRoastin: {
    static int mushies = 0;

    SHOW_TODO(0, "- roast 3 mushies");

    switch (mushies) {
    case 0: {
      SHOW_TODO(1, "  (hold space)");
      SHOW_TODO(2, "  (aim with WASD)");
      SHOW_TODO(3, "  (release space)");
      SHOW_TODO(4, "  [0/3] roasted");
    } break;
    case 1:
      SHOW_TODO(1, "  two more to cook!");
      break;
    case 2:
      SHOW_TODO(1, "  LAST ONE!");
      break;
    }

    /* mushroom v. fireball collision */
    for (int i = 0; onscreen_mush[i]; i++) {
      Mushroom *m = onscreen_mush[i];

      for (int i = 0; i < ARR_LEN(state.fireballs); i++) {
        Fireball *f = state.fireballs + i;

        if (f->ts_fade_out > state.elapsed) {
          Vec2 fpos = fireball_pos(f);

          // use t to stop fireballs that are still "in the air" from hitting
          // mushies float t = fireball_t(f); if (t > 0.8f && mag2(sub2(fpos,
          // m->pos)) < 0.2f)

          if (mag2(sub2(fpos, m->pos)) < 0.2f)
            if (m->stage == MushroomStage_Ripe ||
                m->stage == MushroomStage_Collected) {
              m->stage = MushroomStage_Blasted;
              mushies++;
              if (mushies > 2)
                stage = QuestStage_MushRoastinDone;
            }
        }
      }
    }
  } break;

  case QuestStage_MushRoastinDone: {
    SHOW_TODO(0, "report back to wiz");
    if (wiz_dist < seeing_dist)
      stage = QuestStage_SaveWizFromMushMen;
  } break;

  case QuestStage_SaveWizFromMushMen: {
    SHOW_TODO(0, "- save wiz!");
    SHOW_TODO(1, "- don't die!");

    static Man henchmen[] = {{.pos = {wiz_x + 0.3f, wiz_y - 0.1f},
                              .dir = 0.4f - 0.1f,
                              .hp = 0.2f,
                              .max_hp = 0.2f},
                             {.pos = {wiz_x - 0.3f, wiz_y + 0.1f},
                              .dir = 0.4f + 0.1f,
                              .hp = 0.2f,
                              .max_hp = 0.2f}};

    static uint8_t pissed_off = 0;
    static uint32_t attack_timer = 0;

    /* if you're close to any of the henchmen, one of them prepares to attack*/
    {
      float closest = 1000.0f;
      for (int i = 0; i < ARR_LEN(henchmen); i++) {
        Man *hench = henchmen + i;
        if (hench->hp <= 0.0f)
          continue;

        closest = fminf(closest, mag2(sub2(hench->pos, state.player.man.pos)));
      }

      /* incrementing the attack timer gradually gives you time to prepare */
      if (closest < seeing_dist || pissed_off)
        attack_timer++;

      if (attack_timer > 120) {
        attack_timer = 0;

        /* start a random place in the array, */
        int offset = (int)(state.elapsed * 5000.0f);

        /* but iterate through til we find a living henchman to attack for us */
        for (int i = 0; i < ARR_LEN(henchmen); i++) {
          Man *hench = henchmen + ((offset + i) % ARR_LEN(henchmen));
          if (hench->hp <= 0.0f)
            continue;

          Vec2 start = add2(hench->pos, (Vec2){0, 0.75f});
          Vec2 target = state.player.man.pos;
          fireballs_push(start, target);
          break;
        }
      }
    }

    /* is an exploding fireball close enough to hurt anyone? */
    {
      const float HURTING_DIST = 0.3f;

      for (int i = 0; i < ARR_LEN(state.fireballs); i++) {
        Fireball *f = state.fireballs + i;
        float die_t = fireball_dying_t(f);
        Vec2 pos = fireball_pos(f);

        /* if it's not in the first bit of its dying stage,
         * >>> it's not real and it can't hurt you. <<< */
        if (die_t < 0.05f || die_t > 0.1f)
          continue;

        float player_dist = mag2(sub2(pos, state.player.man.pos));
        if (player_dist < HURTING_DIST) {
          state.player.man.hp -= 0.1f;
          __builtin_memset(f, 0, sizeof(Fireball));
        }

        for (int i = 0; i < ARR_LEN(henchmen); i++) {
          Man *hench = henchmen + i;
          float hench_dist = mag2(sub2(pos, hench->pos));
          if (hench->hp <= 0.0f)
            continue;

          if (hench_dist < HURTING_DIST) {
            hench->hp -= 0.1f;
            pissed_off = 1;
            __builtin_memset(f, 0, sizeof(Fireball));
          }
        }
      }
    }

    /* gotta put them suckers on the screen */
    for (int i = 0; i < ARR_LEN(henchmen); i++) {
      Man *hench = henchmen + i;
      if (hench->hp <= 0.0f)
        continue;

      geo_man(geo, hench, COLOR_DARKMAROON);
    }

    /* if they're all dead, you win */
    int living_henches = 0;
    for (int i = 0; i < ARR_LEN(henchmen); i++) {
      Man *hench = henchmen + i;
      if (hench->hp <= 0.0f)
        continue;

      living_henches += 1;
    }

    if (living_henches == 0) {
      stage = QuestStage_Done;
    }
  } break;

  case QuestStage_Done: {
    SHOW_TODO(0, "- good job!");
  } break;
  }

  /* TODO:
      - lerp wizard dir to face player
      - gradually type in TODO letters?!
  */
}
#undef SHOW_TODO

void man_anim(Man *man, float dt, Vec2 vel) {
  uint8_t going = dot2(vel, vel) > 0.00001f;
  man->anim_damp = lerp(man->anim_damp, going, dt * 9.0f);
  man->anim_prog += 8.4f * man->anim_damp * dt;
  int anim_len = ARR_LEN(state.mf.frames);
  man->anim_prog =
      lerp_round(anim_len, man->anim_prog, going * man->anim_prog, dt * 3.0f);
  man->anim_prog = fmodf(man->anim_prog, anim_len);

  man->dir = lerp_rad(man->dir, atan2f(-vel.y, -vel.x), going * dt * 6.0f);
}

Vec2 man_pos(Man *man, ManPartKind mpk) {
  float q = man->anim_prog;
  float t = q - round_tof(q, 1.0f);

  int len = ARR_LEN(state.mf.frames);
  int i = (int)(q + 0) % len;
  int n = (int)(q + 0 + 1) % len;

  Vec3 a = state.mf.frames[i].pos[mpk];
  Vec3 b = state.mf.frames[n].pos[mpk];
  Vec4 p = {.x = lerp(a.x, b.x, t) * 1.35f,
            .y = lerp(a.y, b.y, t),
            .z = lerp(a.z, b.z, t),
            .w = 1.0f};

  float mx = cosf(man->dir);
  float my = sinf(man->dir);
  Mat4 m = look_at4x4((Vec3){mx, my, 0.75f}, (Vec3){0, 0, 0}, (Vec3){0, 0, 1});
  p = mul4x44(m, p);
  return (Vec2){p.x * 0.5f + man->pos.x, p.y * 0.5f + man->pos.y};
}

void label_push_i(Label *l, int i, Vec2 p) {
  l->drift = (LABEL_TEXT_SIZE * 1.1f) * 3;

  l->x = p.x;
  l->y = p.y + 0.75f;

  float delay = i * 1.66f;
  l->ts_pop_in = state.elapsed + delay;
  l->ts_fade_out = state.elapsed + 5.0f + delay;
  labels_push(*l);
}

void fireballs_push(Vec2 start, Vec2 target) {
  Vec2 delta = sub2(target, start);

  /* enforce range by clipping target */
  float dist = fminf(1.5f, mag2(delta));
  target = add2(start, mul2_f(norm2(delta), dist));

  const float units_per_second = 0.3f;

  Fireball new = {.start = start, .target = target};
  new.ts_spawned = state.elapsed;
  new.ts_charged = new.ts_spawned + 0.3f;
  new.ts_exploded = new.ts_charged + dist *units_per_second;
  new.ts_fade_out = new.ts_exploded + 1.0f;

  for (int i = 0; i < ARR_LEN(state.fireballs); i++) {
    Fireball *f = state.fireballs + i;
    if (f->ts_fade_out <= state.elapsed) {
      *f = new;
      return;
    }
  }
}
