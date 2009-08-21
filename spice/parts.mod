* Component models for interesting devices
*
* Regular diode
*
.model diode d
*
* Zener diodes
*
.model mz56 d(bv=5.6V)
.model mz12 d(bv=12V)
.model mz16 d(bv=16V)
*
* Transistors.  These are placeholders because I can't tell what the
* actual transistors were.  
*
* The CDC 6600 Training Manual (60147400), appendix B, describes the
* basic circuit properties.  It shows the "on" state of the transistor
* as having Ib = 1 mA, Ic = 10 mA, implying a beta of 10.  So that's
* what is used here for the NPN transistor, and for lack of more
* information for the PNP transistor as well.  Note that the DD60
* is an analog design that uses several different transistor types,
* so this is clearly a simplification, hopefully not too serious.
*
.model mnpn npn(level=1, bf=10)
.model mpnp pnp(level=1, bf=10)
*
* Tubes
*
.include 12bh7a.mod
.include 3cx100a5.mod
*
* Coax from mainframe (75 feet)
*
* Specs: capacitance 21.5 pF/foot, loss .045 dB/foot, 
* propagation delay 1.5 ns/foot.
*
* According to an article about lossy transmission line modeling
* by Spectrum Software
* (http://www.spectrum-soft.com/news/spring97/tline.shtm)
* that means the transmission line parameters (per meter) are:
*
* C = 65.6 pF
* L = 371 nH
* R = 2.55 ohm
*
.model coax75 ltra(r=2.55, L=371nH, C=65.6pF, len=22.9)
*
*... but unfortunately that doesn't work...
