
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

#define MAN_FRAME_COUNT (5)
typedef struct {
    ManPartFrame frames[MAN_FRAME_COUNT*2 - 2];
} ManFrames;
static void man_frames_fill(ManFrames *mf) {
  mf->frames[0].time = 0.0;
  mf->frames[1].time = 10.0;
  mf->frames[2].time = 20.0;
  mf->frames[3].time = 30.0;
  mf->frames[4].time = 40.0;
  mf->frames[0].pos[ManPartKind_Elbow_L] = (Vec3) { .x = -0.11455299705266953, .y = 0.02565222978591919, .z = 0.9458134770393372 };
  mf->frames[1].pos[ManPartKind_Elbow_L] = (Vec3) { .x = -0.11455299705266953, .y = -0.005274122580885887, .z = 0.9148871302604675 };
  mf->frames[2].pos[ManPartKind_Elbow_L] = (Vec3) { .x = -0.11455299705266953, .y = 0.02565222978591919, .z = 0.937379002571106 };
  mf->frames[3].pos[ManPartKind_Elbow_L] = (Vec3) { .x = -0.11455299705266953, .y = -0.00808560848236084, .z = 0.9345675110816956 };
  mf->frames[4].pos[ManPartKind_Elbow_L] = (Vec3) { .x = -0.11455299705266953, .y = 0.02565222978591919, .z = 0.9458134770393372 };
  mf->frames[0].pos[ManPartKind_Elbow_R] = (Vec3) { .x = 0.11455299705266953, .y = -0.10757745057344437, .z = 0.9485974311828613 };
  mf->frames[1].pos[ManPartKind_Elbow_R] = (Vec3) { .x = 0.11455299705266953, .y = -0.17786461114883423, .z = 0.9401629567146301 };
  mf->frames[2].pos[ManPartKind_Elbow_R] = (Vec3) { .x = 0.11455299705266953, .y = -0.07102811336517334, .z = 0.9457859396934509 };
  mf->frames[3].pos[ManPartKind_Elbow_R] = (Vec3) { .x = 0.11455299705266953, .y = -0.06821662932634354, .z = 0.9626548290252686 };
  mf->frames[4].pos[ManPartKind_Elbow_R] = (Vec3) { .x = 0.11455299705266953, .y = -0.10757745057344437, .z = 0.9485974311828613 };
  mf->frames[0].pos[ManPartKind_Hand_L] = (Vec3) { .x = -0.09195800125598907, .y = 0.08024269342422485, .z = 0.6921569108963013 };
  mf->frames[1].pos[ManPartKind_Hand_L] = (Vec3) { .x = -0.09195800125598907, .y = 0.08024269342422485, .z = 0.6584190726280212 };
  mf->frames[2].pos[ManPartKind_Hand_L] = (Vec3) { .x = -0.09195800125598907, .y = 0.09711161255836487, .z = 0.6977798938751221 };
  mf->frames[3].pos[ManPartKind_Hand_L] = (Vec3) { .x = -0.09195800125598907, .y = 0.004332557320594788, .z = 0.7202717661857605 };
  mf->frames[4].pos[ManPartKind_Hand_L] = (Vec3) { .x = -0.09195800125598907, .y = 0.08024269342422485, .z = 0.6921569108963013 };
  mf->frames[0].pos[ManPartKind_Hand_R] = (Vec3) { .x = 0.09195800125598907, .y = -0.14248749613761902, .z = 0.6977249383926392 };
  mf->frames[1].pos[ManPartKind_Hand_R] = (Vec3) { .x = 0.09195800125598907, .y = -0.28025034070014954, .z = 0.6864789724349976 };
  mf->frames[2].pos[ManPartKind_Hand_R] = (Vec3) { .x = 0.09195800125598907, .y = -0.14248749613761902, .z = 0.7145938277244568 };
  mf->frames[3].pos[ManPartKind_Hand_R] = (Vec3) { .x = 0.09195800125598907, .y = -0.021593570709228516, .z = 0.7286512851715088 };
  mf->frames[4].pos[ManPartKind_Hand_R] = (Vec3) { .x = 0.09195800125598907, .y = -0.14248749613761902, .z = 0.6977249383926392 };
  mf->frames[0].pos[ManPartKind_Head] = (Vec3) { .x = 2.2734190596376785e-18, .y = -0.020696094259619713, .z = 1.3413537740707397 };
  mf->frames[1].pos[ManPartKind_Head] = (Vec3) { .x = -6.466436462157766e-18, .y = -0.05162244662642479, .z = 1.3019930124282837 };
  mf->frames[2].pos[ManPartKind_Head] = (Vec3) { .x = 1.6491433698020709e-18, .y = 0.035533640533685684, .z = 1.3385423421859741 };
  mf->frames[3].pos[ManPartKind_Head] = (Vec3) { .x = 7.267622096780702e-18, .y = -0.006638657301664352, .z = 1.363845705986023 };
  mf->frames[4].pos[ManPartKind_Head] = (Vec3) { .x = 2.2734190596376785e-18, .y = -0.020696094259619713, .z = 1.3413537740707397 };
  mf->frames[0].pos[ManPartKind_Knee_L] = (Vec3) { .x = -0.059964001178741455, .y = 0.04728153347969055, .z = 0.4469897747039795 };
  mf->frames[1].pos[ManPartKind_Knee_L] = (Vec3) { .x = -0.059964001178741455, .y = 0.024789638817310333, .z = 0.4413667917251587 };
  mf->frames[2].pos[ManPartKind_Knee_L] = (Vec3) { .x = -0.059964001178741455, .y = 0.024789638817310333, .z = 0.4413667917251587 };
  mf->frames[3].pos[ManPartKind_Knee_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.07121962308883667, .z = 0.48164623975753784 };
  mf->frames[4].pos[ManPartKind_Knee_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.11684426665306091, .z = 0.4507198929786682 };
  mf->frames[0].pos[ManPartKind_Knee_R] = (Vec3) { .x = 0.059964001178741455, .y = -0.11144382506608963, .z = 0.4596002697944641 };
  mf->frames[1].pos[ManPartKind_Knee_R] = (Vec3) { .x = 0.059964001178741455, .y = -0.16767355799674988, .z = 0.4230509400367737 };
  mf->frames[2].pos[ManPartKind_Knee_R] = (Vec3) { .x = 0.059964001178741455, .y = -0.013041798956692219, .z = 0.4539772868156433 };
  mf->frames[3].pos[ManPartKind_Knee_R] = (Vec3) { .x = 0.059964001178741455, .y = 0.029549218714237213, .z = 0.47646915912628174 };
  mf->frames[4].pos[ManPartKind_Knee_R] = (Vec3) { .x = 0.059964001178741455, .y = 0.04149162024259567, .z = 0.4596002697944641 };
  mf->frames[0].pos[ManPartKind_Neck] = (Vec3) { .x = -2.7691803299123525e-17, .y = -0.03194205090403557, .z = 1.203590989112854 };
  mf->frames[1].pos[ManPartKind_Neck] = (Vec3) { .x = -3.643165799373836e-17, .y = -0.0656798854470253, .z = 1.164230227470398 };
  mf->frames[2].pos[ManPartKind_Neck] = (Vec3) { .x = -2.7067526782107305e-17, .y = 0.013041744008660316, .z = 1.2064025402069092 };
  mf->frames[3].pos[ManPartKind_Neck] = (Vec3) { .x = -2.394614916010988e-17, .y = -0.03194203972816467, .z = 1.2204599380493164 };
  mf->frames[4].pos[ManPartKind_Neck] = (Vec3) { .x = -2.7691803299123525e-17, .y = -0.03194205090403557, .z = 1.203590989112854 };
  mf->frames[0].pos[ManPartKind_Pelvis] = (Vec3) { .x = -1.3536813371408258e-16, .y = -0.04115667566657066, .z = 0.7186599373817444 };
  mf->frames[1].pos[ManPartKind_Pelvis] = (Vec3) { .x = -1.3474385719706636e-16, .y = -0.0889519453048706, .z = 0.7214714288711548 };
  mf->frames[2].pos[ManPartKind_Pelvis] = (Vec3) { .x = -1.2600399919372908e-16, .y = -0.013041798956692219, .z = 0.7608322501182556 };
  mf->frames[3].pos[ManPartKind_Pelvis] = (Vec3) { .x = -1.2600399919372908e-16, .y = -0.04288286343216896, .z = 0.7608322501182556 };
  mf->frames[4].pos[ManPartKind_Pelvis] = (Vec3) { .x = -1.3536813371408258e-16, .y = -0.04115667566657066, .z = 0.7186599373817444 };
  mf->frames[0].pos[ManPartKind_Sole_L] = (Vec3) { .x = -0.059964001178741455, .y = 0.11923544853925705, .z = 0.13590377569198608 };
  mf->frames[1].pos[ManPartKind_Sole_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.02696186676621437, .z = 0.13590377569198608 };
  mf->frames[2].pos[ManPartKind_Sole_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.055076733231544495, .z = 0.13871526718139648 };
  mf->frames[3].pos[ManPartKind_Sole_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.1989368051290512, .z = 0.1812763512134552 };
  mf->frames[4].pos[ManPartKind_Sole_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.25377780199050903, .z = 0.1657448410987854 };
  mf->frames[0].pos[ManPartKind_Sole_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.25692683458328247, .z = 0.1973993480205536 };
  mf->frames[1].pos[ManPartKind_Sole_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.35814034938812256, .z = 0.2058338075876236 };
  mf->frames[2].pos[ManPartKind_Sole_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.23162344098091125, .z = 0.24800610542297363 };
  mf->frames[3].pos[ManPartKind_Sole_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.09246768802404404, .z = 0.2578890323638916 };
  mf->frames[4].pos[ManPartKind_Sole_R] = (Vec3) { .x = 0.0599643774330616, .y = 0.11981657147407532, .z = 0.17874868214130402 };
  mf->frames[0].pos[ManPartKind_Toe_L] = (Vec3) { .x = -0.059964001178741455, .y = 0.18307998776435852, .z = 0.17526459693908691 };
  mf->frames[1].pos[ManPartKind_Toe_L] = (Vec3) { .x = -0.059964001178741455, .y = 0.039694175124168396, .z = 0.13871526718139648 };
  mf->frames[2].pos[ManPartKind_Toe_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.019347045570611954, .z = 0.13871526718139648 };
  mf->frames[3].pos[ManPartKind_Toe_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.14071522653102875, .z = 0.13348111510276794 };
  mf->frames[4].pos[ManPartKind_Toe_L] = (Vec3) { .x = -0.059964001178741455, .y = -0.197393536567688, .z = 0.13423313200473785 };
  mf->frames[0].pos[ManPartKind_Toe_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.19925013184547424, .z = 0.16524581611156464 };
  mf->frames[1].pos[ManPartKind_Toe_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.31452107429504395, .z = 0.15681135654449463 };
  mf->frames[2].pos[ManPartKind_Toe_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.24423392117023468, .z = 0.19336068630218506 };
  mf->frames[3].pos[ManPartKind_Toe_R] = (Vec3) { .x = 0.0599643774330616, .y = -0.04225126653909683, .z = 0.21357092261314392 };
  mf->frames[4].pos[ManPartKind_Toe_R] = (Vec3) { .x = 0.0599643774330616, .y = 0.17376312613487244, .z = 0.21373754739761353 };
}