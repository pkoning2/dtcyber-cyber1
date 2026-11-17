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

## Checking a module against its page

1. Find the page by LS number in the PDF text layer. Render at 150 dpi, and crop at 400 dpi for dense areas.
2. Check every element and every output, not only the letter inputs and the first and last gates.
3. Confirm each triangle sits on the net the VHDL exports for it.
4. Compile the file with GHDL after cyberdefs.vhd: `ghdl -a --std=93c cyberdefs.vhd <file>.vhd`.
5. When the user corrects a drawing reading, re-check the page before arguing the point.

The current review state is in [REVIEW_STATUS.md](REVIEW_STATUS.md).
