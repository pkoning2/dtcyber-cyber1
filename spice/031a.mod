* NGSpice model -- 031A module for dd60
.subckt 031a in1 in2 in3 in4 out1 out2 out3 out4 vcc vee
D1       out1 n6 mz16
D2       out2 n20 mz16
D3       out3 n16 mz16
D4       out4 n17 mz16
Q1       n6 in1 n2 mpnp
Q2       n20 in2 n4 mpnp
Q3       n3 n9 n8 mpnp
Q4       n13 n9 n10 mpnp
Q5       n16 in3 n12 mpnp
Q6       n17 in4 n14 mpnp
R1       vee out1 1500
R2       n3 n2 820
R3       n4 n3 820
R4       vee out2 1500
R5       n8 vcc 1500
R6       0 n9 1500
R7       n9 vcc 1000
R8       n10 vcc 1500
R9       vee out3 1500
R10      n13 n12 820
R11      n14 n13 820
R12      vee out4 1500
.ends 013a
