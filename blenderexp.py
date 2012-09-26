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
from collections import namedtuple

Face   = namedtuple('Face',   ['v1', 'v2', 'v3'])
Vertex = namedtuple('Vertex', ['co', 'n', 'uv'])

def writeObj(objects):
    faces = []
    for obj in objects:
        faces += collect_faces(obj.data, obj.location)

    faces, vertices = index_vertices(faces)

    # outfile.write("g %s\n" % obj.name)

    for v in vertices:
        outfile.write("vn %f %f %f\n" % v.n)
        if v.uv:
            outfile.write("vt %f %f\n" % v.uv)
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

def to_tuple(vec):
    """Convert Blender vector into tuple, swaps Y and Z"""
    return (vec.x, vec.z, -vec.y)

def collect_faces(mesh, loc):
    """collect data from the given mesh and triangulate it"""

    uv_faces = None
    faces = mesh.faces
    if mesh.uv_textures.active:
        uv_faces = mesh.uv_textures.active.data

    out_faces = []
        
    for i in range(0, len(faces)):
        v1 = mesh.vertices[faces[i].vertices[0]]
        v2 = mesh.vertices[faces[i].vertices[1]]
        v3 = mesh.vertices[faces[i].vertices[2]]
        if len(faces[i].vertices) == 4:
            v4 = mesh.vertices[faces[i].vertices[3]]

        uv1 = uv2 = uv3 = uv4 = None
        if uv_faces:
            uv1 = uv_faces[i].uv1.to_tuple()
            uv2 = uv_faces[i].uv2.to_tuple()
            uv3 = uv_faces[i].uv3.to_tuple()
            if len(faces[i].vertices) == 4:
                uv4 = uv_faces[i].uv4.to_tuple()

        face = Face(Vertex(to_tuple(v1.co + loc), to_tuple(v1.normal), uv1),
                    Vertex(to_tuple(v2.co + loc), to_tuple(v2.normal), uv2),
                    Vertex(to_tuple(v3.co + loc), to_tuple(v3.normal), uv3))

        out_faces.append(face)
       
        if len(faces[i].vertices) == 4:
            face = Face(Vertex(to_tuple(v1.co + loc), to_tuple(v1.normal), uv1),
                        Vertex(to_tuple(v3.co + loc), to_tuple(v3.normal), uv3),
                        Vertex(to_tuple(v4.co + loc), to_tuple(v4.normal), uv4))

            out_faces.append(face)

    return out_faces

with open("/tmp/blender.mod", "w") as outfile:
    outfile.write("# exported by %s\n" % __file__)
    objects = [obj for obj in bpy.data.objects if obj.type == 'MESH']
    writeObj(objects)
    outfile.write("\n# EOF #\n")

print("-- export complete --")

# EOF #
