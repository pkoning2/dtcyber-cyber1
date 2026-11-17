# VHDL review status

Review of the logic modules in this directory against the 60042700 schematic pages. The full per-module report is generated from the review log, not kept here. Conventions are in [CLAUDE.md](CLAUDE.md).

## Fixes applied (2026-11-17, uncommitted)

| Module | Change |
|---|---|
| ak | u19 first input j -> i |
| an | u16 `a => p16` (already in HEAD) |
| um | added `p9` port; u7 `i1 => p9` |
| gd | u5 inverter input p9 -> p8 |
| sm | tp4 and tp5 connections swapped (u4 and u5) |
| ic | triangle 2 export tp3 -> tp2 |
| jg | deleted stray u8 (inverter with no output) |
| hv | added `p3` port, driven by `p3 <= t8` (NAND(P4,P5)) |
| kr | p27_tp7 -> p27_tp6; added `tp2 <= m` (square M from P11) |
| kx | P10 export p10_tp2 -> p10_tp3; P17 export p17_tp5 -> p17_tp4. Page 35 has four triangles (1, 3, 4, 6), so no tp2 or tp5 is needed |
| mr | p5_tp3 -> p5_tp2; p25_tp4 -> p25_tp5 |
| nc | u3 second input g -> h; deleted stray u4 |
| hs | deleted stray u24 |
| kn | added NAND(t13, e) for circle 54 feeding u24; exported `tp6` |
| nn | nnslice gained `tp_a` (t1) and `tp_b` (t5) ports; top exports tp1, tp2, tp5, tp6 |
| we | rewritten from V-Z page 139; compiles |

All edited files and `we.vhd` compile with GHDL after `cyberdefs.vhd` (`/Users/pkoning/bin/ghdl/bin/ghdl -a --std=93c`). This is a syntax and type check only. Nothing has been simulated.

## Retracted or resolved (no change needed)

- pw: both P19 NAND3s have identical inputs (square 51, P19, D) and collapse to one gate.
- mg: P18 = NAND(P1, A) matches; t14 = P1 because `inv2` `y2` equals its input.
- nc: F and G are both NAND(D,E) and collapse to `g`.
- jx: M and P are both NOT P12; tri 4 and P20 share one net.
- it, qi: `inv2` `y2` chains match the page.
- hs, iy, jr, np, rh: full element traces match.
- ab, cm, co, ct, ck, il, hq, aj, hl, qr, cw, gf: earlier findings resolved or retracted.

## Open

- The 40 modules flagged for missing test-point exports still need the duplicate-net check.
- Many OK entries came from earlier pin- and triangle-level passes, not element-by-element traces.
- Simulation has not been run on any model.
