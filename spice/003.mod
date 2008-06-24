* NGSpice model -- 003 module for dd60
.subckt 003 in out vcc vee
*Part     Value
C1       n3 in 22pF
Q1       n3 in n2 mnpn
Q2       n2 n5 n6 mnpn
Q3       vcc 0 n2 mnpn
Q4       0 n3 out mpnp
R1       in vee 3300
R2       n5 0 33000
R3       n5 vee 8200
R4       n6 vee 1000
R5       vcc n3 68000
R6       in out 1000
R7       vcc out 2000
.ends 003
