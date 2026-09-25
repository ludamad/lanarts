"""The red-haired rogue in grey with a bow (spr_classes/rogue2.png)."""
from animations import GOLD as MAGIC                             # noqa: F401

SOURCE = "spr_classes/rogue2.png"
WEAPON_KIND = "bow"
WEAPON = {4: (7, 7), 5: (6, 8), 6: (5, 7), 7: (5, 7), 8: (4, 6), 9: (4, 6), 10: (3, 6), 11: (3, 6),
          12: (3, 7), 13: (3, 7), 14: (4, 7), 15: (5, 7), 16: (6, 8), 17: (6, 8), 18: (7, 9), 19: (8, 8)}
GRIP = (6, 14)
WEAPON_SIDE = "left"
LEG_LEFT = {23: (11, 15), 24: (12, 15), 25: (12, 15), 26: (11, 15), 27: (11, 15), 28: (10, 14),
            29: (9, 14), 30: (9, 15)}
LEG_RIGHT = {23: (19, 23), 24: (19, 22), 25: (19, 22), 26: (19, 22), 27: (19, 23), 28: (20, 23),
             29: (19, 24), 30: (19, 25)}
UPPER_TO_ROW = 22
