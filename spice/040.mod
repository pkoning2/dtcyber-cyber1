* NGspice model -- 040 module for dd60
*
* diff input, diff output, supplies
.subckt 040 p1 p7 p4 p10 vcc vee

D1       0 n3 mz12
Q1       n3 p1 p4 mnpn
Q2       n3 p7 p10 mnpn
R1       n3 vcc 151
R7       n6 p4 136
R13      n6 p10 136
R19      n6 vee 97
.ends 040
