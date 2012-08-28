import Blender

def writeObject(object):
        if object.getType() == "Mesh":
                mesh = object.getData()
                print dir(mesh)
                writeVertices(mesh.verts, object.loc)
                writeFaces(mesh.faces)

def writeVertices(vertices, location):
        outfile.write("%d\n" % len(vertices))
        for i in vertices:
                outfile.write("%f %f %f\n"
                              % (i[0] + location.x,
                                 i[1] + location.y,
                                 i[2] + location.z))

def writeFaces(faces):
        #print "Faces: ", faces
        outfile.write("%d\n" % (len(faces)))
        for i in faces:
                #print "Face: ", i
                #print i.v[0], i.v[1], i.v[2]
                outfile.write("%d %d %d\n" % (i.v[0].index, i.v[1].index, i.v[2].index))


outfile = open("/tmp/blender.mod", "w")
scene = Blender.Scene.getCurrent()

outfile.write ("%d\n" % (len(scene.getChildren())))
for child in scene.getChildren():
        writeObject(child)
        
outfile.close()


# EOF #
