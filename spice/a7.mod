* A7 chassis (deflection/unblank preamps)
*
* Currently this is just one coordinate deflection, no unblank
*
.include 039.mod
.include 029.mod
.include 040.mod
*
* Connections: differential inputs, c1 trimmer, differential outputs,
* vcc and vee.
*
.subckt a7 in1 in2 c1a c1b out1 out2 vcc vee
xm2 in1 in2 0 0 039
*
xm1 in1 in2 c1a c1b m1p4 m1p10 vcc vee 029
c1 c1a c1b 580pF
*
xm4 m1p14 m1p10 out1 out2 vcc vee 040
*
.ends a7
