"""The armoured fighter in teal plate with purple greaves (classes/sprites/fighter.png)."""

SOURCE = "classes/sprites/fighter.png"
# blade held upright on the right, fist and pommel below the crossguard
WEAPON = {1: (21, 23), 2: (21, 23), 3: (21, 23), 4: (21, 23), 5: (22, 24), 6: (22, 24), 7: (22, 24),
          8: (23, 25), 9: (23, 25), 10: (23, 26), 11: (23, 27), 12: (24, 28), 13: (25, 28),
          14: (25, 27), 15: (25, 27), 16: (26, 26)}
GRIP = (26, 12)
WEAPON_SIDE = "right"
ATTACK = [(30, 1, 0), (-90, -1, 0), (-140, -1, 1)]
LEG_LEFT = {20: (9, 13), 21: (9, 13), 22: (9, 13), 23: (10, 13), 24: (9, 13), 25: (10, 13),
            26: (11, 14), 27: (11, 14), 28: (9, 14), 29: (8, 14), 30: (8, 14)}
LEG_RIGHT = {20: (16, 19), 21: (16, 20), 22: (17, 20), 23: (18, 21), 24: (19, 22), 25: (19, 22),
             26: (19, 22), 27: (20, 23), 28: (20, 25), 29: (20, 26), 30: (20, 26)}
UPPER_TO_ROW = 19
