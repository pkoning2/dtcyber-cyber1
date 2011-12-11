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
* Transistors.  
*
.include transistors.mod
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
