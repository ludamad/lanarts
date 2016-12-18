# Python that combines images from spr_* folders and generates Lua to assign appropriate labels.
# Notes on the various resources:
# - Tiles are handled as one image having all the variants of a tile. (TODO)
import os
import sys
from collections import OrderedDict, defaultdict

#os.system("convert -crop 32x32 image.png temporary.png")
print "local Display = require 'core.Display'"
print "local M = nilprotect {}"

SPR_FOLDERS = ["spr_doors", "spr_keys", "spr_rings", "spr_rings/randarts", "spr_weapons", "spr_armour"]
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
        full_id = spr_folder.replace('/', '.') + "." + id
        resources.append(full_id)
        print "M['" + full_id + "'] = Display.image_load '" + filename + "'"
    print ""
resources.sort()

print "M.resource_id_list = {"
for id in resources:
    print "    '" + id + "',"
print "}"

print "return M"