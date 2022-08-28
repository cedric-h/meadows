import bpy

def capFirstLast(s):
    if (s[-2] == '_'):
        return s[:-1].capitalize() + s[-1].upper()
    else:
        return s.capitalize()

enum = '''
typedef enum {
'''

parts = {}
for o in bpy.data.collections.get('parts').all_objects:
    name = 'ManPartKind_' + capFirstLast(o.name)
    enum += '  ' + name + ',\n';
    map = {}
    for x in o.animation_data.action.fcurves:
        for p in x.keyframe_points:
            [t, y] = p.co
            t = str(t)
            if not t in map:
                map[t] = [None]*3
            map[t][x.array_index] = y
    parts[name] = map

enum += '  ManPartKind_COUNT,\n'
enum += '} ManPartKind;\n'

frames = '''
typedef struct {
    float time;
    Vec3 pos[ManPartKind_COUNT];
} ManPartFrame;

#define MAN_FRAME_COUNT (%s)
typedef struct {
    ManPartFrame frames[MAN_FRAME_COUNT];
} ManFrames;\n''' % len(map)

fill = ''
fill += f'static void man_frames_fill(ManFrames *mf) {{\n'
times = ''
posses = ''

for i, k in enumerate(list(parts.values())[0]):
    fill += f'  mf->frames[{i}].time = {k};\n'
for name in parts:
    map = parts[name]
    for i, k in enumerate(map):
        x, y, z = map[k]
        fill += f'  mf->frames[{i}].pos[{name}] = (Vec3) {{ .x = {x}, .y = {y}, .z = {z} }};\n'
fill += f'}}\n'

print(enum + frames + fill)
open('man.h', 'w+').write(enum + frames + fill)