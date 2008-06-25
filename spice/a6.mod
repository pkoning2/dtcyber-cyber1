* A6 chassis, deflection signal path only
*
.include 620.mod
.include c19.mod
.include 457a.mod
.include 003.mod
.include 031a.mod
.include 015.mod

* Connections: coordinate diff. in, c1/c2 trimmer connections, 
* differential output (to a7), vcc, vee.
*
.subckt a6 in1 in2 in3 in4 c1a c1b c2a c2b out1 out2 out3 out4 vcc vee
*
* X coordinate path
*
xy018 in1 in2 y018p3 y018p11 vcc vee 620
* r3, pot, r4
r3 y018p3 y018p11 290
*
xy509 y018p3 y018p11 c1a c1b y509p5 y509p11 vcc vee c19
c3 c1a c1b 50pF
*
xy511 y509p5 0 0 0 y509p11 0 0 0 y511p1 y511p7 457a
*
xy300 y511p1 y300p7 vcc vee 003
xy301 y511p7 y301p7 vcc vee 003
*
* Y coordinate path
*
xy019 in3 in4 y019p3 y019p11 vcc vee 620
* r1, pot, r2
r1 y019p3 y019p11 290
*
xy508 y019p3 y019p11 c2a c2b y508p5 y508p11 vcc vee c19
c4 c2a c2b 50pF
*
xy513 y508p5 0 0 0 y508p11 0 0 0 y513p1 y513p7 457a
*
xy302 y513p1 y302p7 vcc vee 003
xy303 y513p7 y303p7 vcc vee 003
*
xy400 y300p7 y301p7 y302p7 y303p7 y400p1 y400p4 y400p9 y400p11 vcc vee 031a
*
xy401 y400p1 y400p4 y400p9 y400p11 y401p2 y401p5 y401p8 y401p11 vee 015
*
* technically speaking this next one should be an 015a
xy404 y401p2 y401p5 y401p8 y401p11 out1 out2 out3 out4 vee 015
*
.ends a6
