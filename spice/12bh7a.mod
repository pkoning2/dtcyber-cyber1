* 12BH7A Triode PSpice Model
*
* Connections: Plate
*              | Grid
*              | | Cathode
*              | | |
.SUBCKT 12BH7A A G K
B1  b1  0  v=(v(a,k) / 200.)*log(1. + exp(200. * (1. / 16.5 + (v(g,k) / sqrt (300. + v(a,k) * v(a,k))))))
r1 b1 0 1g
rg g 0 1g
Ba  A  K  I=((2. * uramp (v(B1))^1.2) / 800.)
* capacitances with shield
Cgk G  K  3.3p
Cgp G  A  2.4p
Cpk A  K  2.0p
.ENDS 12BH7A
