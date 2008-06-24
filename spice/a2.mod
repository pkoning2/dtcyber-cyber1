* NGSpice model -- A2 chassis for dd60
.subckt a2 in1 in2 c1a c1b c4a c4b out1 out2 v600 v2k
R1       0 in1 1M
R2       0 in2 1M
R3       n6 v600 40k
R4       0 n1 1500
R5       n2 v600 40k
R6       0 c1b 1500
R7       0 n9 25k
R8       0 n5 25k
R9       c4a c4b 3900
R10      n10 v2k 15k
R11      c4b n14 5k
R12      n11 v2k 15k
R13      n18 c4a 5k
R27      c1a n1 5k
R29      n15 n14 1500
R30      n1 c1b 470
* R32      2500
R32a     n15 0 1250
R32b     n17 0 1250
R33      n18 n17 1500
R37      n11 out1 220
R38      n10 out2 220
* tubes: anode grid kathode
XV1a      n2 in1 c1b 12BH7A
XV1b      v600 n2 n5 12BH7A
XV2a      n6 in2 n1 12BH7A
XV2b      v600 n6 n9 12BH7A
XV3       n11 n5 c4b 3CX100A5
XV4       n10 n9 c4a 3CX100A5
.ends a2
