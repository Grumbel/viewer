#!/usr/bin/env python2

import sys
from PIL import Image

if len(sys.argv) != 2:
    print "Usage: %s FILENAME" % sys.argv[0]
    print "Takes an image in the form:"
    print "+--------+"
    print "|  []    |"
    print "|[][][][]|"
    print "|  []    |"
    print "+--------+"    
    print "Splits it up into individual images."
else:
    img = Image.open(sys.argv[1])

    width, height = img.size

    w = width / 4
    h = height / 3

    spec = [("up", 1, 0, False),
            ("dn", 1, 2, False),
            ("ft", 1, 1, False),
            ("bk", 3, 1, False),
            ("lf", 0, 1, False),
            ("rt", 2, 1, False)]

    for name, x, y, rot in spec:
        sub = img.crop((x*w, y*h, x*w+w, y*h+h)) # .transpose(Image.FLIP_LEFT_RIGHT)
        # if rot: 
        #     sub = sub.transpose(Image.ROTATE_180)
        sub.save(name + ".png", "PNG")
    
# EOF #
