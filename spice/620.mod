* NGSpice model -- 620 module for dd60
.subckt 620 in1 in2 out1 out2 vcc vee
C1       0 vee .01uF
C2       vcc 0 .01uF
C3       n5 n4 33pF
C4       n13 n12 33pF
d1       n7 0 mz56
Q1       n8 n2 0 mpnp
Q2       n7 n8 n5 mpnp
Q3       n16 n10 0 mpnp
Q4       n7 n16 n13 mpnp
R1       n3 in1 91
R2       n2 in1 1000
R3       n4 n2 1000
R4       n8 vee 6800
R5       n4 n5 470
R6       n5 vcc 2200
R7       vee n7 910
R8       vee n7 910
R9       0 n5 1000
R10      out1 n5 180
R11      vcc 0 2200
R12      n11 in2 91
R13      n10 in2 1000
R14      n12 n10 1000
R15      n16 vee 6800
R16      n12 n13 470
R17      n13 vcc 2200
R18      0 n13 1000
R19      out2 n13 180
.ends 620
