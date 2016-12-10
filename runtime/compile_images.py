# Python that combines images from spr_* folders and generates Lua to assign appropriate labels.
# Notes on the various resources:
# - Tiles are handled as one image having all the variants of a tile. (TODO)
import os
import sys
from collections import OrderedDict, defaultdict

class Sprite:
    def __init__(self, name, parts, type):
        self.name = name
        self.parts = parts
        self.type = type
    def output_reconstruction(self):
        pass

#os.system("convert -crop 32x32 image.png temporary.png")
print "local Display = require 'core.Display'"
print "local M = nilprotect {}"

SPR_FOLDERS = ["spr_doors", "spr_keys", "spr_rings", "spr_rings/randarts", "spr_weapons"]
used_ids = defaultdict(lambda: None)
resources = []
for spr_folder in SPR_FOLDERS:
    print "-- " + spr_folder + " resource loads:"
    for basename in os.listdir(spr_folder):
        if "." not in basename: # No file extension? Skip.
            continue
        filename = spr_folder + "/" + basename
        id = basename.split(".")[0].replace('/', '.')
        if used_ids[id] is not None:
            print "Error, clashing file names " + used_ids[id] + " and " + filename + ". Must have different base names."
            sys.exit(1)
        used_ids[id] = True
        resources.append(spr_folder + "." + id)
        print "M['" + spr_folder + "." + id + "'] = Display.image_load '" + filename + "'"
    print ""
resources.sort()

print "M.resource_id_list = {"
for id in resources:
    print "    '" + id + "',"
print "}"

print "return M"
