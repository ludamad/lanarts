"""The hooded rogue in a blue cloak with a bow (spr_classes/rogue1.png)."""
from animations import GOLD as MAGIC                             # noqa: F401

SOURCE = "spr_classes/rogue1.png"
WEAPON_KIND = "bow"
WEAPON = {4: (5, 5), 5: (4, 6), 6: (3, 5), 7: (3, 5), 8: (2, 4), 9: (2, 4), 10: (1, 4), 11: (1, 4),
          12: (1, 5), 13: (1, 5), 14: (2, 5), 15: (3, 6), 16: (4, 9), 17: (5, 9), 18: (5, 6)}
GRIP = (7, 16)
WEAPON_SIDE = "left"
LEG_LEFT = {29: (6, 11), 30: (7, 12)}
LEG_RIGHT = {29: (20, 23), 30: (19, 23)}
UPPER_TO_ROW = 28
