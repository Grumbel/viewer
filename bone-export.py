##  Blender Export Script
##  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.

import bpy
import sys

def vec3(v):
    return "%6.2f %6.2f %6.2f" % (v.x, v.y, v.z)

def vec4(v):
    return "%6.2f %6.2f %6.2f %6.2f" % (v.x, v.y, v.z, v.w)

def mat3(m):
    m = m.transposed()
    return "\n%s\n%s\n%s\n" % (vec3(m[0]), vec3(m[1]), vec3(m[2]))

def mat4(m):
    m = m.transposed()
    return "\n%s\n%s\n%s\n%s\n" % (vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(m[3]))

def export_armature(f, obj):
    f.write("object %s\n" % obj.name)
    armature = obj.data
    for bone in armature.bones:
        # _local is in armature space, the other in bone space
        f.write("  bone %s\n" % bone.name)
        if bone.parent: f.write("    parent     %s\n" % bone.parent.name) 
        f.write("    head       %s\n" % vec3(bone.head))
        f.write("    tail       %s\n" % vec3(bone.tail))
        f.write("    head_local %s\n" % vec3(bone.head_local))
        f.write("    tail_local %s\n" % vec3(bone.tail_local))
        f.write("    matrix %s\n" % mat3(bone.matrix)) # a 3x3 matrix
        f.write("    matrix_local %s\n" % mat4(bone.matrix_local)) # a 4x4 matrix
        # bone.children

# bpy.data.objects[0].data.vertices[1].groups -> VertexGroupElement
# bpy.data.objects[0].data.vertices[1].groups[0] -> weight

# armature.pose.bones -> [PoseBone]
# >>> bpy.data.objects[0].pose.bones[1].matrix
# >>> bpy.data.objects[0].pose.bones[1].parent
# >>> bpy.data.objects[0].pose.bones[2].matrix_basis

# >>> bpy.data.objects["Cube"].parent_type
# 'BONE'
# >>> bpy.data.objects["Cube"].parent
# bpy.data.objects["Armature"]
# >>> bpy.data.objects["Cube"].parent_bone
# 'Bone.002'
# >>> bpy.data.objects["Cube"].parent_type
# 'BONE'

objects = [obj for obj in bpy.data.objects if obj.type == 'ARMATURE']
print("-" * 80)
f = sys.stdout
for obj in objects:
    export_armature(f, obj)

# EOF #
