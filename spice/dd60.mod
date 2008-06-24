* DD60 display model
*
* For now this models only the character shape signal path, for
* the X coordinate only.
*
.include parts.mod
.include a2.mod
.include a6.mod
.include a7.mod
*
xa6 in1 in2 a6c1a a6c1b a6out1 a6out2 vcc vee a6
c1a6 a6c1a a6c1b 100pF
*
xa7 a6out1 a6out2 a7c1a a7c1b a7out1 a7out2 vcc vee a7
c1a7 a7c1a a7c1b 580pF
*
xa2 a7out1 a7out2 c1a c1b c4a c4b def1 def2 v600 v2k a2
c1 c1a c1b 3055pF
c4 c4a c4b 300pF
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
eplus v1 0 vctrl 0 1
eminus vref v2 vctrl 0 -1
t1 v1 0 in1 0 z0=75 td=2ns
t2 v2 0 in2 0 z0=75 td=2ns
*
* Test signal
*
vctrl vctrl 0 pwl(0 0 100ns 0 105ns 2 200ns 2 205ns 0 300ns 0)
*
*** Run the test
*
.tran 2ns 300ns
.save vctrl in1 in2 def1 def2
.end
