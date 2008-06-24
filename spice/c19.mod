* NGSpice model -- C19 module for dd60
.subckt c19 in1 in2 trim1 trim2 out1 out2 vcc vee
C1       n3 n10 22pF
Q1       n3 in1 trim1 mpnp
Q2       vee n3 n4 mpnp
Q3       trim2 in2 n10 mpnp
Q4       n12 n10 vee mpnp
R1       vee n3 2200
R2       trim1 vcc 4700
R3       0 trim1 1500
R4       n10 n3 6800
R5       out1 n4 4700
R6       vcc n4 6800
R7       out1 0 2700
R8       n10 vee 2200
R9       vcc trim2 4700
R10      trim2 0 1500
R11      out2 n12 4700
R12      vcc n12 6800
R13      0 out2 2700
.ends c19
