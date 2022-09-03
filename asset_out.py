import bpy
import bmesh

out = '---- \n'

def obj_material(o):
    return o.material_slots[o.data.polygons[0].material_index].material

def material_base_color(material):
    nodes = material.node_tree.nodes
    # Get a principled node
    principled = next(n for n in nodes if n.type == 'BSDF_PRINCIPLED')
    # Get the slot for 'base color'
    base_color = principled.inputs['Base Color'] #Or principled.inputs[0]
    # Get its default value (not the value from a possible link)
    return base_color.default_value

def obj_base_color(o):
    return material_base_color(obj_material(o))

for o in bpy.data.collections.get('parts').all_objects:
    out += 'uint16_t cave_' + o.name + '_idx[] = {\n'
    for p in o.data.polygons:
        out += '  ' + ', '.join([str(int(x)) for x in p.vertices]) + ',\n'
    out += '};\n'
    
    out += 'Vec2 cave_' + o.name + '_vpos[] = {\n'
    for x in o.data.vertices:
        out += f'  {{ {x.co[0]}, {x.co[1]} }},\n'
    out += '};\n\n'

out += 'typedef struct {\n'
out += '  uint16_t *idx; size_t idx_len; \n'
out += '  float z;\n'
out += '  Vec2 *vpos; \n' # size_t vpos_len; \n'
out += '  Color color;\n'
out += '} ExportedGeo;\n'

out += 'ExportedGeo exported_geos[] = {\n'
for o in bpy.data.collections.get('parts').all_objects:
    out += '  {\n'
    out += '    .idx = cave_' + o.name + '_idx,\n'
    out += '    .idx_len = ARR_LEN(cave_' + o.name + '_idx),\n'
    
    out += '    .z = ' + str(o.data.vertices[0].co[2]) + ',\n'
    
    out += '    .vpos = cave_' + o.name + '_vpos,\n'
    
    out += '    .color = { ' + ', '.join([str(x) for x in obj_base_color(o)]) + ' },\n'
    # out += 'ARR_LEN(cave_' + o.name + '_vpos), '
    out += '  },\n'
out += '};\n'

print(out);