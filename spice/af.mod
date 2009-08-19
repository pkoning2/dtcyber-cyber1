* NGspice model -- 6612 AF module 
*
* input stage (one input, one output, 6 volt supply)
.model diode d
.subckt afslice in out v6
r1 in n1 150
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
