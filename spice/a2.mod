* NGSpice model -- A2 chassis for dd60
.subckt a2 in1 in2 c1a c1b c4a c4b out1 out2 v600 v2k
R1       0 in1 1Meg
R2       0 in2 1Meg
R3       nr3 v600 40k
L3       nr3 n6 20uH
C3       n6 v600 10pF
R4       0 n1 1500
R5       nr5 v600 40k
L5       nr5 n2 20uH
C5       n2 v600 10pF 
R6       0 c1b 1500
R7       0 nr7 25k
L7       nr7 n9 20uH
C7       0 n9 10pF
R8       0 nr8 25k
L8       nr8 n5 20uH
C8       0 n5 10pF
R9       c4a c4b 3900
R10      nr10 v2k 15k
L10      nr10 n10 20uH
C10      n10 v2k 10pF
R11      c4b nr11 5k
L11      nr11 n14 20uH
C11      c4b n14 10pF
R12      nr12 v2k 15k
L12      nr12 n11 20uH
C12      n11 v2k 10pF
R13      nr13 c4a 5k
L13      nr13 n18 20uH
C13      c4a n18 10pF
R27      c1a n1 2500
R29      nr29 n14 1500
L29      nr29 n15 20uH
C29      n15 n14 10pF
R30      n1 c1b 470
* R32      2500
R32a     n15 0 1250
R32b     n17 0 1250
R33      nr33 n17 1500
L33      nr33 n18 20uH
C33      n18 n17 10pF
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
