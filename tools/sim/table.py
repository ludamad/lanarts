import re, sys, collections, statistics
def load(f):
    d = collections.defaultdict(list)
    for l in open(f):
        m = re.match(r"SIM class=(\S+) level=(\d+) players=(\d+) seed=(\d+) mon=(.*?) frames=(\d+) maxhp=(\S+) dmg=(\S+) left=(\d+) left_hp=(\d+)/(\d+)", l)
        if not m: continue
        cl, L, P, seed, mon, fr, maxhp, dmg, left, lhp, lmax = m.groups()
        dmg = sum(map(int, dmg.split("/"))); maxhp = int(maxhp.split("/")[0])
        lhp, lmax = int(lhp), int(lmax)
        # Extrapolate unfinished fights: assume damage scales with fraction of enemy HP removed
        killed_frac = 1.0
        if int(left) > 0 and lmax > 0:
            # fraction of total enemy HP still left; approximate total as max of left and single-monster max
            killed_frac = max(0.2, 1 - lhp / lmax)
        d[(cl, int(L), int(P), mon)].append((dmg / killed_frac, maxhp, int(fr), int(left)))
    return d
if __name__ == "__main__":
    d = load(sys.argv[1]); cl = sys.argv[2]; P = int(sys.argv[3])
    mons = sorted({k[3] for k in d})
    print(f"{cl} P={P}: mean party dmg per monster / (P*maxHP) at level L; * = timeout")
    print(f"{'monster':18s}" + "".join(f"{L:>7d}" for L in range(1, 9)))
    for m in mons:
        row = ""
        for L in range(1, 9):
            v = d.get((cl, L, P, m))
            if not v: row += "      -"; continue
            x = statistics.mean(a for a, b, c, e in v) / (P * v[0][1])
            row += f"{x:6.2f}" + ("*" if any(e for *_, e in v) else " ")
        print(f"{m:18s}{row}")
