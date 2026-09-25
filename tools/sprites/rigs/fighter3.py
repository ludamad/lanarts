"""The knight with the red cape and heater shield (classes/sprites/fighter3.png)."""
from rig import rect

SOURCE = "classes/sprites/fighter3.png"
WEAPON = rect(5, 0, 9, 20)                     # upright sword and the fist under its crossguard
GRIP = (8, 17)
WEAPON_SIDE = "left"
# a diagonal slash: the hand goes up and back beside the helm with the blade tipped behind, sweeps
# across the chest with the blade level, and finishes low in front with the blade driven down right
ATTACK = [(-35, -1, 0, 1, -7), (90, 1, 0, 3, -5), (135, 1, 1, 5, 2)]
LEG_LEFT = {22: (11, 15), 23: (11, 15), 24: (11, 15), 25: (11, 15), 26: (11, 15), 27: (11, 15),
            28: (10, 14), 29: (9, 14), 30: (9, 13)}
LEG_RIGHT = {22: (18, 22), 23: (18, 22), 24: (18, 22), 25: (18, 22), 26: (18, 22), 27: (18, 22),
             28: (19, 23), 29: (19, 23), 30: (20, 24)}
UPPER_TO_ROW = 21
KEEP_UPPER = [{22: (23, 31), 23: (23, 31), 24: (24, 31), 25: (24, 31), 26: (24, 31), 27: (25, 31)}]  # shield
SHADOW = [(9, 29, 17), (11, 30, 15), (13, 31, 11)]
