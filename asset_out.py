import bpy
import bmesh

out = '---- \n'

for o in bpy.data.collections.get('parts').all_objects:
    out += 'uint16_t pot_' + o.name + '_idx[] = {\n'
    for p in o.data.polygons:
        out += '  ' + ', '.join([str(int(x)) for x in p.vertices]) + ',\n'
    out += '};\n'
    
    out += 'Vec2 pot_' + o.name + '_vpos[] = {\n'
    for x in o.data.vertices:
        out += f'  {{ {x.co[0]}, {x.co[1]} }},\n'
    out += '};\n\n'

print(out)