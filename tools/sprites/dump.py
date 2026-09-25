"""Prints a 32x32 sprite as a grid of symbols (one per colour, darkest first) with a column ruler,
for tracing layer regions. Usage: dump.py path/to/sprite.png"""
import sys
from PIL import Image
from collections import Counter
im=Image.open(sys.argv[1]).convert("RGBA")
px=[[im.getpixel((x,y)) for x in range(32)] for y in range(32)]
cols=Counter(p for r in px for p in r if p[3]>0)
syms="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&*+=?@"
order=sorted(cols, key=lambda p: (p[3]<255, 0.3*p[0]+0.59*p[1]+0.11*p[2]))
m={p:syms[i] for i,p in enumerate(order)}
print(sys.argv[1], len(order),"colours")
print(" ".join(f"{m[p]}=#{p[0]:02x}{p[1]:02x}{p[2]:02x}{'' if p[3]==255 else '/'+str(p[3])}" for p in order))
print("   "+"".join(str(x%10) for x in range(32)))
for y,r in enumerate(px):
    print("%2d "%y+"".join(m[p] if p[3]>0 else "." for p in r))
