# Lanarts combat philosophy: movement as skill

The soul of Lanarts is **fast, reactive movement**: enemies react to where you go,
and you dodge by *being elsewhere*. So skill is precise quick movement, and the
combo system (**Flow**) rewards sustaining it. This doc is the design north star and
the per-class philosophy, written *after* reading each kit (not from bot telemetry
alone — see the measurement caveat).

## The one rule: play at your edge

Every class has a **dangerous edge** where it is most effective and most exposed.
Skilled play *lives at that edge*; weak play retreats from it (turtling,
face-tanking, spamming from safety). Flow is the scoreboard:

- **Build** flow with clean dodges and **grazes** (closer to the hit edge = more).
- **Bleed** flow if you stop moving; **break** ~60% on a hit.
- **Spend/convert** flow into power.

Design test for any ability/tuning change: *does it pull the player toward their
edge and pay off the flow earned there?* If it rewards retreating from the edge, it
is wrong.

## Per-class philosophy (grounded in the actual kits)

- **Fighter** — *the brawler.* Berserk (rage commit window), Dash Attack, Power
  Strike (charged AoE knockback). Mace, durable. Edge = sustained point-blank
  presence. Berserk = a Flow **spender** (trade defense for a burst at a moment you
  judge safe); Dash = re-engage/weave. Today it face-tanks (perfect still −49 micro,
  3% flow); it needs the dash to *build* flow so the brawler dances instead of plants.

- **Templar** — *the bulwark.* Only Power Strike + Dash Attack, deliberately fewer
  tools, most durable melee. Edge = holding a contested spot but repositioning on
  telegraphs. Power Strike resets spacing (knockback). Its identity is *anchoring a
  lane*, so its flow reward should come from holding ground near pressure while still
  weaving — not from planting.

- **Rogue** — *the blur, but its kit is empty.* It has **only Dash Attack** (Hide-in-
  Sight is commented out), 100 HP, high HP-regen, dagger. Its weakness/boredom is
  literally a one-ability kit. Fix is **give Rogue a real identity** built on
  mobility: the dash should be a deep flow engine (dash-through = weave + strike), and
  it likely needs a second tool (a reposition/finisher) so hit-and-run has a payoff.

- **Dicebinder** — *the gambler.* Dash + Power Strike + Berserk **plus the dice
  system** (RNG windows that grant temporary buffs/cooldown trims). Edge = exploit a
  hot roll while sustaining flow. Dice payoffs should bias toward *movement* (a roll
  that refreshes the dash / opens a weave window) so the gamble feeds the soul.

- **Necromancer** — *the army commander* (NOT a regen-tank). str 0 / mpregen 0; its
  loop is **HP is the currency for an army**: Summon Dark Aspect spends HP for pets,
  Skeleton Army bursts ~25 one-hit skeletons, Baleful Regeneration refills HP on an
  ~800f cooldown (with an "Exhausted" lockout), and Pain (free range-50 aura) is the
  filler attack. The real skill is the **HP economy + army positioning + surviving
  regen's downtime**. Edge = the regen-funded risk margin: dive and spend HP boldly,
  *but only within regen's window*. Regen should fund aggression, not hide mistakes —
  judge it regen-adjusted so "tanked it" never reads as healthy.

- **Pyrocaster** — *the commit pyromancer* (NOT a pure kiter). Fire Bolt pokes at
  range, **Ring of Flames** is a self-aura power window (stand in the ring, enemies
  burn, kills spread fire), **Dragonform** is a *slow* but immense burst ultimate (4×
  exploding bolts). Edge = choosing **when to commit** Ring/Dragonform (high power,
  low mobility) vs. safe poke, then surviving the commit. Flow should reward weaving
  *through* the committed window (you're slow in Dragonform but can still micro), so
  the payoff for committing is real but earned by movement.

- **Stormcaller** — *the storm dancer (the model).* Energy Spear (wall-bounce,
  persistent), Chain Lightning (bounces between enemies — reward clusters/lineups),
  Tornado (zones). Tankier mage. Already deep: positions enemies, uses walls, mixes
  abilities, converts movement to wins. Leave its loop; it is the bar.

## How abilities interact with Flow (patterns)

- **Movement abilities (Dash)** = flow *builders*: weaving through danger/enemies
  builds flow; never pure escapes.
- **Commit abilities (Berserk, Dragonform, Power Strike, channels)** = flow
  *spenders* / risk windows: powerful but they root/slow/expose you, so you cash flow
  at a judged-safe moment; a greedy commit that eats a hit breaks the chain.
- **Positional abilities (Chain Lightning, Energy Spear bounce, Ring zones)** reward
  arranging the battlefield; flow can sharpen them at high tier.
- **Sustain (Necro regen)** = graze insurance that funds bold movement, not a tank
  button.

## Measured skill depth — and its caveat

A 4-profile sweep (sloppy→human→skilled→perfect) gives a composite skill-depth
(perfect-vs-sloppy spread on win/micro/flow/graze): Stormcaller +0.63, Dicebinder
+0.47, Rogue +0.39, Fighter +0.37, Pyrocaster +0.33, **Necromancer +0.15, Templar
+0.12**.

**Caveat (important):** the bot expresses *movement/dodge* skill, not a class's
deeper game. So a low score can mean either "shallow design" OR "the bot doesn't play
this class's real loop." Necromancer's flat score is partly the bot spamming
Pain+regen instead of running the HP→army economy; Pyrocaster's low win despite high
micro is partly the bot not committing Ring/Dragonform well. So: use the bot to
measure **movement-skill** rigorously; reason about **economy/commit** skill from the
kit and validate with draw smokes, not bot win rate alone.

Genuinely actionable from the data (movement-skill, which the bot does measure):
- **Fighter/Templar low ceiling** — even perfect can't weave (face-tank-locked). The
  **dash→flow weave** is the clean fix: make the dash a movement-skill flow engine so
  skilled melee dances. (Also directly helps Rogue, whose only ability is the dash.)
- **Stormcaller** stays the bar; don't flatten it.

## How we keep it honest

`run_class_interestingness_experiment.sh` + `aggregate_class_interestingness.py`
(per-class playstyle) and `aggregate_skill_depth.py` (sloppy→perfect spread) score
every change against: *did skilled movement get more rewarded, and did
playing-from-safety get less?* — while remembering the bot only measures the movement
layer.
