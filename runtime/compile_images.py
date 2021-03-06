# Python that combines images from spr_* folders and generates Lua to assign appropriate labels.
# Notes on the various resources:
# - Tiles are handled as one image having all the variants of a tile. (TODO)
import os
import sys
from collections import OrderedDict, defaultdict

#os.system("convert -crop 32x32 image.png temporary.png")
print "local Display = require 'core.Display'"
print "local M = nilprotect {}"

SPR_FOLDERS = [
    "spr_books",
    "spr_bonuses",
    "spr_classes",
    "spr_classes/icons",
    "spr_doors",
    "spr_keys",
    "spr_fonts",
    "spr_rings",
    "spr_menu",
    "spr_weapons",
    "spr_gamepad",
    "spr_armour",
    "spr_effects",
    "spr_spells",
    "spr_amulets",
    "spr_belts",
    "spr_legwear",
    "spr_boots",
    "spr_scrolls",
    "spr_runes",
    "spr_gates",
    "spr_enemies",
    "spr_enemies/undead",
    "spr_enemies/animals",
    "spr_enemies/demons",
    "spr_enemies/humanoid",
    "spr_enemies/demonspawn",
    "spr_enemies/draco",
    "spr_enemies/dragons",
    "spr_enemies/bosses"
]

# Expand wherever a 'randarts' subfolder is present:
for spr in SPR_FOLDERS:
    try:
        os.lstat(spr + "/randarts")
        # Did we not get an exception?
        SPR_FOLDERS.append(spr + "/randarts")
    except OSError:
        pass

used_ids = defaultdict(lambda: None)
resources = []
for spr_folder in SPR_FOLDERS:
    print "-- " + spr_folder + " resource loads:"
    for basename in os.listdir(spr_folder):
        if basename.startswith("."): # Hidden file? Skip
            continue
        if "." not in basename: # No file extension? Skip.
            continue
        filename = spr_folder + "/" + basename
        id = basename.split(".")[0].replace('/', '.')
        # if used_ids[id] is not None:
        #     sys.stderr.write("Error, clashing file names " + id + " and " + filename + ". Must have different base names.\n")
        #     sys.exit(1)
        used_ids[id] = True
        full_id = spr_folder.replace('/', '.') + "." + id
        resources.append(full_id)
        if len(filename.split(".")) > 3:
            assert filename.split(".")[2] == "dir"
            dims = filename.split(".")[1]
            print "M['" + full_id + "'] = Display.directional_create(Display.images_load '" + filename + "%" + dims +"')"
        elif len(filename.split(".")) > 2:
            dims = filename.split(".")[1]
            print "M['" + full_id + "'] = Display.animation_create(Display.images_load '" + filename + "%" + dims +"', 0.1)"
        else:
            print "M['" + full_id + "'] = Display.image_load '" + filename + "'"
        print "yield_point()"
    print ""
resources.sort()

print "M.resource_id_list = {"
for id in resources:
    print "    '" + id + "',"
print "}"

print "return M"
