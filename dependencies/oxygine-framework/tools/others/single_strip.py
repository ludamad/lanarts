#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import unicode_literals, print_function

import sys
import Image

image = sys.argv[1]
cols = int(sys.argv[2])
rows = int(sys.argv[3])

src = Image.open(image)
fw = src.size[0] / cols
fh = src.size[1] / rows

dest = Image.new("RGBA", (fw * cols * rows, fh))
w = 0
for row in range(rows):
    for col in range(cols):
        frame = src.crop((fw * col, fh * row, fw * (col + 1), fh * (row + 1)))
        dest.paste(frame, (w, 0))
        w += fw

dest.save("anim.png")
print("cols = %d" % (rows * cols, ))
