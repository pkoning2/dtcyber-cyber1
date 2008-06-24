* NGspice model -- 039 module for dd60
*
* inputs: p4 p5 p11 p10
.subckt 039 p4 p5 p11 p10
R1       p4 n7 91
R2       n7 0 1k
R3       n7 0 1k
R4       n7 0 1.2k
R5       n7 0 1.2k
R6       p5 n7 91
R7       p11 n6 91
R8       n6 0 1k
R9       n6 0 1k
R10      n6 0 1.2k
R11      p10 n6 91
R12      n6 0 1.2k
.ends 039
