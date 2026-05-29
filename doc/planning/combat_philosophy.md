# Lanarts combat philosophy: movement as skill

The soul of Lanarts is **fast, reactive movement**: enemies react to where you go,
and you dodge by *being elsewhere*. So skill is precise quick movement, and the
combo system (**Flow**) rewards sustaining it. This doc sets the design north star
and the per-archetype philosophy for how abilities interact with Flow.

## The one rule everything serves: play at your edge

Every archetype has a **dangerous edge** where it is most effective and most
exposed. Skilled play *lives at that edge*; unskilled play retreats from it
(turtling, face-tanking, spamming from safety). Flow is the scoreboard for living
at your edge:

- **Build** flow with clean dodges and **grazes** (the closer to the hit edge, the
  more — see Flow v2). You can only graze a lot if you stay at your edge.
- **Bleed** flow if you stop moving; **break** ~60% if you get hit.
- **Spend/convert** flow into power (empowered openings now; more below).

So the design test for any ability or tuning change is: *does it pull the player
toward their edge and pay off the flow they earn there?* If it rewards retreating
from the edge (a safe nuke, a face-tank stance, a spam button), it is wrong.

Baseline data motivating this (per-class flow%, the share of frames riding the
combo; bot-limited so read relatively): Pyrocaster 10%, Stormcaller 8%, Rogue 7%,
Dicebinder 6%, Templar 4%, **Fighter 2%, Necromancer 2%**. The classes that win by
*not* moving (Fighter face-tank 66%/-46 micro; Necro spam+regen 79%/-17) have the
*lowest* skill expression. That inversion — winning while expressing the least
skill — is the core problem. Skill depth (sloppy→perfect spread) is the metric we
use to confirm a class rewards skill; flat = shallow = needs rework.

## Close combat (Fighter, Templar, Rogue, Dicebinder) — "Dance in the danger"

- **Edge:** point-blank. You must be in melee to deal damage, which is the most
  dangerous place to be. The skill is staying there without dying — weaving
  through and around enemies and their telegraphs.
- **Movement fantasy:** the dash is a *dodge-and-strike*, not an escape — dash
  *through* a sweep gap to both avoid it and hit. Melee should build flow fastest
  (always near danger → grazes everywhere) and lose it hardest (one mistake at
  point-blank = a hit = break). High risk, high flow.
- **Ability ↔ Flow:**
  - **Dash / gap-closers** are the melee flow engine: dashing through an enemy is
    a guaranteed graze + flow; at high flow the dash refreshes faster, so weaving
    chains ("continue and continue"). This is the fix for melee's 2% flow — give
    them a movement tool that *builds* flow so dancing beats face-tanking.
  - **Commit abilities (Berserk, Power Strike, Charge)** are flow *spenders* / risk
    windows: they root or expose you for a burst, so you cash flow at a moment you
    judge safe. Greedy commits that eat a hit break the chain — risk/reward.
  - Sub-identities: **Rogue** = hit-and-run (dash in, graze, dash out; flow from
    pure mobility). **Fighter** = sustained brawler (longer dances, bigger flow
    bursts). **Templar** = bulwark (can hold a line, but flow still demands weaving,
    never standing). **Dicebinder** = its dice windows should bias toward movement
    payoffs (e.g., a roll that refreshes the dash).

## Ranged (Pyrocaster, Stormcaller) — "Stutter-step on the edge"

- **Edge:** the *minimum safe distance*. Far away is safe but low DPS; the skill is
  kiting as close as you dare while still landing shots.
- **Movement fantasy:** stutter-step — fire, micro-reposition, graze the return
  fire, fire again. The graze is dodging the enemy's reach at the lip of your kite.
- **Ability ↔ Flow:**
  - **Skillshots** reward positioning; **Flow empowers them** — at high flow a bolt
    pierces/chains or hits harder. This converts clean kiting into *more damage*,
    fixing Pyrocaster (highest micro +32 but lowest impact, 26% win): a clean kiter
    should be rewarded with burst, not just survival.
  - Flow must reward *staying at the edge*, not turtling at max range — tie flow
    gain to grazing (only possible near the enemy's reach), so backing off to
    safety quietly bleeds your combo. The greed/safety dial is the skill.
  - **Ring of Flames / Tornado / channels** are edge-holding spenders: powerful but
    they tether you to a spot, so you commit flow to a position you must then
    protect with footwork.

## Necromancer — "Conduct, don't tank" (the reframe)

- **The problem:** today Necro is a regen face-tank + Pain-spam — the *least*
  movement-skill class (2% flow, -17 micro, 25% death, yet 79% win). Regen
  currently **masks** mistakes (anti-skill).
- **The reframe:** make regen a **skill enabler, not a crutch.** Necro's edge is
  the *regen-funded risk margin*: because it heals, it can dance *closer* and graze
  *more aggressively* than anyone — regen is "graze insurance," not a reason to
  stand still. So Necro should have the boldest movement, paid for by sustain.
- **Ability ↔ Flow:**
  - **Pain** (the spam button) should reward *position*, not repetition — it is a
    range aura, so flow/payoff should scale with hitting from the optimal lane,
    making good positioning beat mashing.
  - **Minions/auras** create positional puzzles: thread your pets' lanes and your
    own; flow rewards weaving the combined formation under pressure.
  - **Regen ↔ Flow loop:** at high flow, regen converts toward offense (e.g., a
    portion of overheal becomes burst), so taking *bold, near-miss* positioning is
    rewarded, while passive turtling just sits at low flow. This turns the
    survivability that currently hides mistakes into the thing that funds risk.
  - We must keep judging Necro with **regen-adjusted** damage (already telemetered)
    so "survived by tanking" never reads as healthy.

## Measured skill depth (4-profile sweep: sloppy → human → skilled → perfect)

Composite skill-depth (how much perfect beats sloppy across win/micro/flow/graze):
Stormcaller +0.63, Dicebinder +0.47, Rogue +0.39, Fighter +0.37, Pyrocaster +0.33,
**Necromancer +0.15, Templar +0.12**. Read on two axes:

- **Depth** = does skill change the outcome? **Ceiling** = how good is perfect play?
- **Stormcaller** — deep on every axis; the model. Leave its identity, use it as the bar.
- **Pyrocaster** — *deepest micro* (sloppy −19 → perfect +46) but win is flat/negative
  (30% → 25%): skilled movement isn't converted to results. Fix = **flow → burst**.
- **Fighter** — decent depth but **low ceiling**: even perfect averages −49 micro and 3%
  flow. Face-tank-locked. Fix = **dash → flow** to raise the ceiling so skilled melee can
  actually weave to positive micro.
- **Templar** — **shallow AND low ceiling** (flat, perfect still −34). Needs both: a weave
  tool (depth) and a reason to hold-and-reposition rather than plant (ceiling).
- **Necromancer** — **shallowest** (+0.15): win 25 → 70 then flat, micro flat. It spam+regen
  wins at every skill level. The "conduct, don't tank" reframe is the highest-priority rework.
- Dicebinder/Rogue — good depth, low power (Rogue wins ~10%); reward the movement they already
  express with more impact rather than reworking identity.

Rework priority from the data: **(1) Necromancer reframe (shallowest), (2) Fighter/Templar
dash→flow ceiling-raise, (3) Pyrocaster flow→burst payoff.** Don't touch Stormcaller's loop.

## How we measure it (so philosophy stays honest)

- **Flow%/graze%** — does the class sustain the movement combo? (Edge-living.)
- **Skill depth** — sloppy→perfect spread on win/micro/flow/graze. Deep = skill
  matters; flat = spam/face-tank wins. The redesign target is to *raise depth* for
  Fighter/Necro (currently shallow) without flattening the ranged classes that
  already express skill.
- **Face-tank/turtle reliance** — win rate with negative micro / low flow flags a
  class winning *away from its edge*; that is the bad outcome to design out.

The experiment harness (`run_class_interestingness_experiment.sh` +
`aggregate_skill_depth.py`) produces these per class, so every ability/tuning
change is validated against "did skilled movement get more rewarded, and did
playing-from-safety get less rewarded?"
