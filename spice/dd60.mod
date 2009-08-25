* DD60 display model
*
* For now this models only the character shape signal path (X and Y)
*
.include parts.mod
.include a2.mod
.include a6.mod
.include a7.mod
.include af.mod
.include diff.mod
*
*
xa6 in1 in2 in3 in4 a6c1a a6c1b a6c2a a6c2b a6out1 a6out2 a6out3 a6out4 vcc vee a6
* c1/c2 : 10 to 100
c16 a6c1a a6c1b 10pF
c26 a6c2a a6c2b 10pF
*
xa7 a6out1 a6out2 a6out3 a6out4 a7c1a a7c1b a7c2a a7c2b a7out1 a7out2 a7out3 a7out4 vcc vee a7
* c1/c2: 110 to 580
c17 a7c1a a7c1b 10pF
r1a a7c1a a7hc 2500
r1b a7c1b a7hc 2500
r7 a7c1a a7hc 180
c27 a7c2a a7c2b 10pF
r2a a7c2a a7vc 2500
r2b a7c2b a7vc 2500
r8 a7c2a a7vc 180
*
xa2x a7out1 a7out2 c1a c1b c4a c4b def1 def2 v600 v2k a2
* c1/c2: 1400 to 3055
* c3/c4: 55 to 300
c1 c1a c1b 1000pF
c4 c4a c4b 50pF
xa2y a7out3 a7out4 c2a c2b c3a c3b def3 def4 v600 v2k a2
c2 c2a c2b 1000pF
c3 c3a c3b 50pF
*
* Make single ended output for ease of interpretation
ex 0 x def1 def2 1
ey 0 y def3 def4 1
*
* supplies
*
vcc vcc 0 20v
vee 0 vee 20v
v6 v6 0 6v
v600 v600 0 600v
v2k v2k 0 2kv
*
* 6612 D/A output
*
xafx ix1 ix2 ix3 ix4 ix5 ix6 x1 v6 af
xafxb bx1 bx2 bx3 bx3 bx5 bx6 x2 v6 af
xafy iy1 iy2 iy3 iy4 iy5 iy6 y1 v6 af
xafyb by1 by2 by3 by3 by5 by6 y2 v6 af
*
* cable from 6612
*
r1 x1 0 75
r2 x2 0 75
r3 y1 0 75
r4 y2 0 75
* Simulate line loss with a lumped resistor
rl1 x1 x1a 8
rl2 x2 x2a 8
rl3 y1 y1a 8
rl4 y2 y2a 8
t1 x1a 0 in1 0 z0=75 td=112ns
t2 x2a 0 in2 0 z0=75 td=112ns
t3 y1a 0 in3 0 z0=75 td=112ns
t4 y2a 0 in4 0 z0=75 td=112ns
*
* Test signals
*
* Control signals: 0 and 1 for logic level inputs to AF modules
*
.include vctrl.mod
*
* Generate true and complement signals for the AF modules
*
xx1 vx1 ix1 bx1 diff
xx2 vx2 ix2 bx2 diff
xx3 vx3 ix3 bx3 diff
xx4 vx4 ix4 bx4 diff
xx5 vx5 ix5 bx5 diff
xx6 vx6 ix6 bx6 diff
xy1 vy1 iy1 by1 diff
xy2 vy2 iy2 by2 diff
xy3 vy3 iy3 by3 diff
xy4 vy4 iy4 by4 diff
xy5 vy5 iy5 by5 diff
xy6 vy6 iy6 by6 diff
*
*** Options
.option method=gear
*
*** Run the test
*
.tran .1ns 2800ns
*
.end
