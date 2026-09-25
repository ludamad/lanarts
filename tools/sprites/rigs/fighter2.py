"""The barbarian with the two-handed sword and red loincloth (classes/sprites/fighter2.png)."""

SOURCE = "classes/sprites/fighter2.png"
# blade raised up and to the left from both fists at the centre; the crossguard crosses the grip
WEAPON = {2: (3, 4), 3: (3, 5), 4: (3, 6), 5: (4, 7), 6: (5, 8), 7: (6, 9), 8: (7, 10), 9: (8, 11),
          10: (9, 12), 11: [(9, 13), (16, 17)], 12: [(11, 14), (16, 16)], 13: [(12, 14), (16, 16)],
          14: (15, 15), 15: (12, 14)}
GRIP = (15, 13)
WEAPON_SIDE = "left"
ATTACK = [(-35, -1, 0), (80, 1, 0), (140, 1, 1)]
LEG_LEFT = {24: (9, 14), 25: (9, 14), 26: (9, 12), 27: (8, 11)}
LEG_RIGHT = {24: (20, 24), 25: (20, 25), 26: (21, 24), 27: (22, 25), 28: (23, 26)}
UPPER_TO_ROW = 23
