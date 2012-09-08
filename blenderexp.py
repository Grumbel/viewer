import bpy

def writeObject(object):
        if object.type == 'MESH':
                mesh = object.data
                writeVertices(mesh.vertices, object.location)
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

def writeFaces(faces):
        outfile.write("%d\n" % (len(faces)))
        for i in faces:
                outfile.write("%d %d %d\n" % (i.vertices[0], i.vertices[1], i.vertices[2]))


outfile = open("/tmp/blender.mod", "w")

outfile.write("%d\n" % (len(bpy.data.objects)))
for child in bpy.data.objects:
    writeObject(child)
        
outfile.close()


# EOF #
