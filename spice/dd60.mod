* DD60 display model
*
* For now this models only the character shape signal path (X and Y)
*
.include parts.mod
.include a2.mod
.include a6.mod
.include a7.mod
*
xa6 in1 in2 in3 in4 a6c1a a6c1b a6c2a a6c2b a6out1 a6out2 a6out3 a6out4 vcc vee a6
c16 a6c1a a6c1b 100pF
c26 a6c2a a6c2b 100pF
*
xa7 a6out1 a6out2 a6out3 a6out4 a7c1a a7c1b a7c2a a7c2b a7out1 a7out2 a7out3 a7out4 vcc vee a7
c17 a7c1a a7c1b 580pF
c27 a7c2a a7c2b 580pF
*
xa2x a7out1 a7out2 c1a c1b c4a c4b def1 def2 v600 v2k a2
c1 c1a c1b 3055pF
c4 c4a c4b 300pF
xa2y a7out3 a7out4 c2a c2b c3a c3b def3 def4 v600 v2k a2
c2 c2a c2b 3055pF
c3 c3a c3b 300pF
*
* Make single ended output for ease of interpretation
ex 0 x def1 def2 1
ey 0 y def3 def4 1
*
* supplies
*
vcc vcc 0 20v
vee 0 vee 20v
v600 v600 0 600v
v2k v2k 0 2kv
*
* connection to 6612
*
vref vref 0 5v
ex1 x1 0 vx 0 1
ex2 x2 vref 0 vx 1
ey1 y1 0 vx 0 1
ey2 y2 vref 0 vx 1
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
* Test signal
*
vx vx 0 pwl(0ns 0 10ns 0 15ns 2 110ns 2 115ns 0 210ns 0)
vy vy 0 pwl(0ns 1 10ns 1 15ns 2 110ns 2 115ns 0 210ns 0)
*
*** Run the test
*
.tran 2ns 600ns
*
.end
