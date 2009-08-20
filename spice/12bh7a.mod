* 12BH7A Triode PSpice Model
*
* Connections: Plate
*              | Grid
*              | | Cathode
*              | | |
* This is the model described by Koren, with the following parameters:
*    mu   25
*    ex   1.2
*    kg1  160
*    kp   95
*    kvb  100
*
.SUBCKT 12BH7A A G K
B1  b1  0  v=(v(a,k) / 95.)*log(1. + exp(95. * (1. / 25 + (v(g,k) / sqrt (100. + v(a,k) * v(a,k))))))
r1 b1 0 1g
rg g 0 1g
Ba  A  K  I=((2. * uramp (v(B1))^1.2) / 160.)
* capacitances with shield
Cgk G  K  3.3p
Cgp G  A  2.4p
Cpk A  K  2.0p
.ENDS 12BH7A
