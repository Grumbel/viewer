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

def writeObject(obj):
    mesh = obj.data
    writeVertices(mesh.vertices, obj.location)
    writeFaces(mesh.faces)

def writeVertices(vertices, location):
    outfile.write("%d\n" % len(vertices))
    for i in vertices:
        outfile.write("%f %f %f\n"
                      % (i.normal.x,
                         i.normal.y,
                         i.normal.z))

        outfile.write("%f %f %f\n"
                      % (i.co.x + location.x,
                         i.co.y + location.y,
                         i.co.z + location.z))

def writeFaces(faces_data):       
    # collect data from Blender and triangulate it
    faces = []
    for i in faces_data:
        if len(i.vertices) == 3:
            faces.append((i.vertices[0], i.vertices[1], i.vertices[2]))
        elif len(i.vertices) == 4:
            faces.append((i.vertices[0], i.vertices[1], i.vertices[2]))
            faces.append((i.vertices[0], i.vertices[3], i.vertices[2]))
        else: 
            print("unhandled number of faces: %d" % len(i.vertices))
    
    # write data to file
    outfile.write("%d\n" % len(faces))
    for face in faces:
        outfile.write("%d %d %d\n" % face)


outfile = open("/tmp/blender.mod", "w")

outfile.write("%d\n" % (len(bpy.data.objects)))
objects = [obj for obj in bpy.data.objects if obj.type == 'MESH']
for child in objects:
    writeObject(child)
    
outfile.close()
print("-- export complete --")

# EOF #
