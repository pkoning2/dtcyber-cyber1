* 3CX100A5 model by Paul Koning
*
* Pins   A  Anode
*        G  Grid
*        K  Cathode
*
* This is the model described by Koren, with the following parameters:
*    mu   100
*    ex   1.25
*    kg1  53
*    kp   400

.SUBCKT 3CX100A5 A G K
*
B1  b1  0  v=(v(a,k) / 400.)*log(1. + exp(400. * (1. / 100. + (v(g,k) / sqrt (6000. + v(a,k) * v(a,k))))))
r1 b1 0 1g
rg g 0 1g
Ba  A  K  I=((2. * uramp (v(B1))^1.25) / 53.)
Bg  G  K  I=uramp(.07+ (V(g,k) - 10.)/83.3)+(7.+ V(g,k)/2.)/(V(a,k) + 70.)
*
* Capacitances
*
Cgk	G	K	6.3pF
Cga	A	G	2.0pF
Cak	A	K	.035pF

.ENDS 
