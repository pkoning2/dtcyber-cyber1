* NGspice model -- 6612 AF module 
*
* input stage (one input, one output, 6 volt supply)
* 
* This is modeled as two stages, because the driver of the AF input
* is a twisted pair driver, not a simple logic level.  So the model
* includes that driver along with the AF input transistor.
.subckt afslice in out v6
r11 in n11 150
qd n12 n11 0 mnpn
r12 v6 n12 470
r13 n12 n13 120
*d11 n13 0 diode
r14 n12 tp 56
r1 tp n1 150
q1 n2 n1 0 mnpn
r2 n2 v6 330
r3 n2 out 100
d1 n2 n3 diode 
d2 n3 n4 diode
d3 n4 0  diode
.ends afslice
*
*
* complete module (6 inputs, 1 output, 6 volt supply)
*
.subckt af in1 in2 in3 in4 in5 in6 out v6
x1 in1 n1 v6 afslice
x2 in2 n1 v6 afslice
x3 in3 n1 v6 afslice
x4 in4 n1 v6 afslice
x5 in5 n1 v6 afslice
x6 in6 n1 v6 afslice
r1 n1 v6 120
q1 n2 n1 out mnpn
r2 n2 v6 56
c1 n2 0 1uF
.ends af
