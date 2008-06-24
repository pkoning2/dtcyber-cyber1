* NGSpice model -- 015 module for dd60
.subckt 015 in1 in2 in3 in4 out1 out2 out3 out4 vee
C1       vee 0 .1uF
Q1       vee in1 out1 mpnp
Q2       vee in2 out2 mpnp
Q3       vee in3 out3 mpnp
Q4       vee in4 out4 mpnp
R1       0 out1 4700
R2       0 out2 4700
R3       0 out3 4700
R4       0 out4 4700
.ends 015
