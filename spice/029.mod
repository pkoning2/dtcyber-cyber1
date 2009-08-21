* NGspice model -- 029 module for dd60
*
* diff input, trimmer cap & gain control, diff output
*
.subckt 029 p1 p7 p8 p2 p10 p4 vcc vee
Q1A      n5 p1 n2 MNPN
Q1C      n13 p7 n10 MNPN
Q2A      0 n5 n4 MPNP
Q2C      0 n13 n12 MPNP
Q3A      p10 n4 p2 MPNP
Q3C      p4 n12 p8 MPNP
Q4A      p2 p5 n6 MPNP
Q4C      p8 p11 n14 MPNP
R1       n17 vcc 250
R1A      vee n2 1k
R1C      vee n10 1k
R2A      0 n5 1.5k
R2C      0 n13 1.5k
R3A      n5 vcc 1.8k
R3C      n13 vcc 1.8k
R4A      n4 vcc 2.2k
R4C      n12 vcc 2.2k
R5A      vee p10 1.2k
R5C      vee p4 1.2k
R6A      0 p10 2.2k
R6C      0 p4 2.2k
R7A      n6 vcc 330
R7C      n14 vcc 330
R8A      vee p5 4.7k
R8C      vee p11 4.7k
R31      n17 p5 250
R32      p11 n17 250
.ends 029
