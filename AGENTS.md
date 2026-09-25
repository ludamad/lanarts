# Notes for coding agents

## Commit messages

Write them the way the maintainer does:

- One short line, no body. Say what changed, not how: `Add stun effect to berserk`,
  `Lower cooldown on fighter ult`, `Fix graghs placement + crypt placement`.
- Several small changes can share a line, separated by semicolons:
  `Cheaper randarts; Pain bonus lvl2+; lower saturation grass`.
- No trailing period, no prefixes like `feat:`, no bullet lists.
- No attribution to AI tools: no `Co-Authored-By` trailers, no "Generated with" lines.

## Building and checking changes

- `./run.sh` builds (CMake presets in `CMakePresets.json`) and runs the game; `./run.sh --headless --build`
  builds the headless binary used by tests and simulations.
- `ctest --preset headless` runs the unit tests.
- Gameplay-neutral changes (refactors, dependency updates) should leave map generation and
  combat untouched: compare `tools/sim` runs before and after for the same seeds (see
  `tools/sim/README.md`).
