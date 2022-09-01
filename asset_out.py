import bpy
import bmesh

out = '---- \n'

for o in bpy.data.collections.get('parts').all_objects:
    out += 'uint16_t cave_' + o.name + '_idx[] = {\n'
    for p in o.data.polygons:
        out += '  ' + ', '.join([str(int(x)) for x in p.vertices]) + ',\n'
    out += '};\n'
    
    out += 'float cave_' + o.name + '_z = ' + str(o.data.vertices[0].co[2]) + ';\n'
    
    out += 'Vec2 cave_' + o.name + '_vpos[] = {\n'
    for x in o.data.vertices:
        out += f'  {{ {x.co[0]}, {x.co[1]} }},\n'
    out += '};\n\n'

out += 'typedef struct {\n'
out += '  uint16_t *idx; size_t idx_len; \n'
out += '  float z;\n'
out += '  Vec2 *vpos; \n' # size_t vpos_len; \n'
out += '} ExportedGeo;\n'

out += 'ExportedGeo exported_geos[] = {\n'
for o in bpy.data.collections.get('parts').all_objects:
    out += '  { '
    out += ' cave_' + o.name + '_idx, '
    out += ' ARR_LEN(cave_' + o.name + '_idx), '
    
    out += 'cave_' + o.name + '_z, '
    
    out += 'cave_' + o.name + '_vpos, '
    # out += 'ARR_LEN(cave_' + o.name + '_vpos), '
    out += '},\n'
out += '};\n'

print(out);