* A6 chassis, deflection signal path (one coordinate)
*
.include 620.mod
.include c19.mod
.include 457a.mod
.include 003.mod
.include 031a.mod
.include 015.mod

* Connections: coordinate diff. in, c1 trimmer connections, 
* differential output (to a7), vcc, vee.
*
.subckt a6 in1 in2 c1a c1b out1 out2 vcc vee
xy018 in1 in2 y018p3 y018p11 vcc vee 620
* r3, pot, r3
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
xy400 y300p7 y301p7 0 0 y400p1 y400p4 nc12 nc13 vcc vee 031a
*
xy401 y400p1 y400p4 0 0 y401p2 y402p5 nc22 nc23 vee 015
*
* technically speaking this next one should be an 015a
xy404 y402p2 y402p5 0 0 out1 out2 nc32 nc33 vee 015
*
.ends a6
