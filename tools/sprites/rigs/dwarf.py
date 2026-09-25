"""The dwarf in mail with the red-hafted axe (spr_enemies/humanoid/dwarf.png)."""

SOURCE = "spr_enemies/humanoid/dwarf.png"
# the axe hangs from the fist at the top of the haft, head down at the bottom left
WEAPON = {15: (7, 9), 16: (6, 9), 17: (5, 7), 18: (7, 7), 19: (7, 7), 20: (7, 7), 21: (6, 7),
          22: (6, 7), 23: (5, 7), 24: (5, 6), 25: (5, 6), 26: (2, 7), 27: (1, 7), 28: (1, 7),
          29: (1, 7), 30: (1, 7)}
GRIP = (7, 16)
WEAPON_SIDE = "left"
# overhead chop: axe head swung up above the fist, then brought down across the body to the right
# (he stands at the left edge, so a swing to the left would leave the canvas)
ATTACK = [(160, -1, 0), (230, 1, 0), (300, 1, 1)]
CAST_ANGLE = 180                               # axe raised head-up to the sky
CAST_RAISE = 2
DROOP = -20
LEG_LEFT = {27: (10, 15), 28: (9, 15), 29: (8, 14), 30: (8, 14)}
LEG_RIGHT = {27: (16, 21), 28: (16, 22), 29: (17, 22), 30: (17, 22)}
UPPER_TO_ROW = 26
