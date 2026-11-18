# CDC 6600 cordwood VHDL: working notes

These notes apply to every file in this directory. They describe how CDC 6600 schematic symbols map to the VHDL, so the models can be checked against the 60042700 Printed Circuit Manual pages (A-J, K-U and V-Z_1 PDFs). Keep this file current when a convention changes.

## Reading the schematic

- A circle or square with one input is an inverter. With several inputs it is a NAND stage.
- A bubble on a line is an ordinary input pin feeding fan-out. Outputs are never used as inputs. A bubble splits the line into two segments, each with its own arrowhead, one for each gate it feeds.
- A triangle is a front-panel test point. Its number matches the triangle on the page, not the order of the ports.
- A bubble inside a circle at a coax input is a cxreceiver (y = NOT a). Coax drivers are squares; cxdriver y = NOT a.
- Letters are fan-out labels. The same letter at two places is the same signal.

## VHDL conventions (cyberdefs.vhd)

- gN entities: `y` = NAND, `y2` = AND (`y` negated, one more stage delay).
- `inv`: `y` = NOT a.
- `inv2` is two inverters in series. `y` = NOT a (first stage). `y2` = a (second stage), which EQUALS THE INPUT. Earlier review passes misread this several times, so check every use of `inv2` `y2`.
- `rsflop`: `s` and `r` are active low. If s = '0' then q = 1, elsif r = '0' then q = 0.
- RS flops and latches are modelled as units on purpose. The cross-connected gates inside are not expanded.
- Pull-ups to +6 V are constant '0'.

## Netlist collapse rule

- Gates with identical inputs collapse into one VHDL signal (fan-out split). Example: NC F and G are both NAND(D,E). HS and PW show the same pattern.
- A duplicate-net triangle needs only one test point, placed on the shared net.
- A triangle on an internal slice net is not exported unless the slice has a test-point port.
- Port names joined by underscores (e.g. `p10_tp2`, `p23_p25_p26_p28`) are schematic outputs x, y, z that share one driver and one net, collapsed into a single port to optimize the VHDL. Do not expect separate ports for each name, and do not flag the individual names as undriven.
- A bare name next to an underscore-joined name (e.g. `p10` beside `p10_tp2`) is a duplicate that should be removed, not a separate net.

## Port defaults

- Inputs declared with `:= '1'` (for example `kuslice.e`, `mqslice.i2`, `qkslice.k2/k3`, `czslice.i2`, `qhslice.b/d/e`, `qmslice.b/k2/k3`, `rvslice.i2`) are intentional: the page shows that input unused or tied to the active level.
- Before calling a port unconnected, read the entity declaration for `:= '1'` defaults. A port that is not mapped in an instance takes its default, so the gate it feeds may have fewer real inputs than the instance appears to have. Always compute slice outputs with the defaults applied (for example `qhslice` with b = d = e = '1' reduces to NOT a, NOT c, and the NAND of those). An unmapped port with a default is not a defect.

## Latch models

- `latch2` ANDs its two clocks (`clki <= clk and clk2`). When a page shows a gate such as `NAND(x, AND(P15, P16))` feeding a latch, the AND is the latch clock and is modelled by `clk`/`clk2`; the set gate itself may be the `d` input.
- `latchd4` is a variant of the basic latch with two gated data pairs (`d`/`e` and `d2`/`e2`, ORed). Its `clk` is active high, the same as the other latch types.
- The schematic set/reset pattern is not a plain D latch. With CLK high (P11 high for the `we` pattern, which asserts R and enables D into S), Q follows D and the not-Q output is also asserted until CLK falls. After the falling edge, not-Q becomes NOT(Q) as usual. The `latch` model in cyberdefs.vhd behaves this way. It is planned to be replaced with a plain level-sensitive latch, on the assumption that the not-Q behaviour during CLK high does not matter to the systems that use these modules.
- `latch22` has two data inputs (`d`, `d2`) and two clocks (`clk`, `clk2`). Use it where the page shows the set network built twice (for example P3 with NOT P6 at the top of JB, NOT P25 with P26 at the bottom).

## Checking a module against its page

1. Find the page by LS number in the PDF text layer. Render at 150 dpi, and crop at 400 dpi for dense areas.
2. Check every element and every output, not only the letter inputs and the first and last gates.
3. Confirm each triangle sits on the net the VHDL exports for it.
4. Compile the file with GHDL after cyberdefs.vhd: `ghdl -a --std=93c cyberdefs.vhd <file>.vhd`.
5. When the user corrects a drawing reading, re-check the page before arguing the point.

The current review state is in [REVIEW_STATUS.md](REVIEW_STATUS.md).
