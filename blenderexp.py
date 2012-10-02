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
from mathutils import Matrix, Vector
from collections import namedtuple

Face   = namedtuple('Face',   ['v1', 'v2', 'v3'])
Vertex = namedtuple('Vertex', ['co', 'n', 'uv', 'bones'])

# "B2OGL * vec" results in OpenGL coordinates
B2OGL = Matrix(((1, 0,  0, 0),
                (0, 0, -1, 0),
                (0, 1,  0, 0),
                (0, 0,  0, 1)))

def vec3(v):
    """Convert Blender vector into tuple, swaps Y and Z"""
    # return (v.x, v.z, -v.y)
    return (v.x, v.y, v.z)

def writeObj(obj):
    faces = collect_faces(obj.data)
    faces, vertices = index_vertices(faces)

    outfile.write("o %s\n" % obj.name)
    outfile.write("loc %f %f %f\n" % vec3(obj.location))

    for v in vertices:
        outfile.write("vn %f %f %f\n" % v.n)
        if v.uv:
            outfile.write("vt %f %f\n" % v.uv)

        bones = list(v.bones)
        while len(bones) < 4:
            bones.append((0, 0.0))

        # FIXME: should only discard least important bone
        while len(bones) > 4:
            bones.pop()

        bone_index  = [g for g, w in bones]
        bone_weight = [w for g, w in bones]
        bone_weight = [w / sum(bone_weight) for w in bone_weight]

        outfile.write("bi %d %d %d %d\n" % tuple(bone_index))
        outfile.write("bw %f %f %f %f\n" % tuple(bone_weight))
        
        outfile.write("v %f %f %f\n" % v.co)

    for f in faces:
        outfile.write("f %d %d %d\n" % (f.v1, f.v2, f.v3))

def index_vertices(faces):
    # collect vertices
    vertices = {}
    for face in faces:
        vertices[face.v1] = None
        vertices[face.v2] = None
        vertices[face.v3] = None
        
    # number the vertices
    i = 0
    for k in vertices.keys():
        vertices[k] = i
        i += 1

    # replace vertices with index
    out_faces = []
    for face in faces:
        out_faces.append(Face(vertices[face.v1],
                              vertices[face.v2],
                              vertices[face.v3]))

    return out_faces, list(vertices.keys())

def collect_faces(mesh):
    """collect data from the given mesh and triangulate it"""

    uv_faces = None
    faces = mesh.faces
    if mesh.uv_textures.active:
        uv_faces = mesh.uv_textures.active.data

    out_faces = []

    # bpy.data.objects[0].data.vertices[1].groups -> VertexGroupElement
    # bpy.data.objects[0].data.vertices[1].groups[0] -> weight
        
    for face in faces:
        num_vertices = len(face.vertices)
        v = [mesh.vertices[face.vertices[i]] for i in range(0, num_vertices)]

        if uv_faces:
            uv = [tuple(uv_faces[face.index].uv[i]) for i in range(0, num_vertices)]
        else:
            uv = [None, None, None, None]

        bones = [[], [], [], []]
        for i, vert in enumerate(v):
            for j, g in enumerate(mesh.vertices[vert.index].groups):
                bones[i].append((g.group, g.weight))
            bones[i] = tuple(bones[i])
        bones = tuple(bones)

        out_faces.append(
            Face(Vertex(vec3(v[0].co), vec3(v[0].normal), uv[0], bones[0]),
                 Vertex(vec3(v[1].co), vec3(v[1].normal), uv[1], bones[1]),
                 Vertex(vec3(v[2].co), vec3(v[2].normal), uv[2], bones[2])))
       
        if num_vertices == 4:
            out_faces.append(
                Face(Vertex(vec3(v[0].co), vec3(v[0].normal), uv[0], bones[0]),
                     Vertex(vec3(v[2].co), vec3(v[2].normal), uv[2], bones[2]),
                     Vertex(vec3(v[3].co), vec3(v[3].normal), uv[3], bones[3])))

    return out_faces

with open("/tmp/blender.mod", "w") as outfile:
    outfile.write("# exported by %s\n" % __file__)
    objects = [obj for obj in bpy.data.objects if obj.type == 'MESH']
    for obj in objects:
        writeObj(obj)
    outfile.write("\n# EOF #\n")

print("-- export complete --")

# EOF #
