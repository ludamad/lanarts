-- The bosses defined in enemies/bosses/: each file defines one, with its spells. Their sprites,
-- animations and spell effects are built by tools/bosses/build.py.
BOSSES = {"RoyalJelly", "StormEye", "Ereshkigal", "BoneWyrm"}

for name in *BOSSES
    require "enemies.bosses.#{name}"
