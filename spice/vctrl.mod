* include file for dd60 control signals to AF modules
*
* Signals are vx1 through vx6 and vy1 through vy6.  To represent
* input value n, the v lines for 1 to n are set (1) and from n+1 
* are reset (0).
*
* 0 0 0
* 0 2 100
* 0 4 200
* 2 6 300
* 4 6 400
* 6 4 500
* 6 2 600
* 4 0 700
* 2 0 800
* 0 2 900
*
vx1 vx1 0 pwl (0ns 0 300ns 0 305ns 1 900ns 1 905ns 0 1000ns 0)
ex2 vx2 0 vx1 0 1
vx3 vx3 0 pwl (0ns 0 400ns 0 405ns 1 800ns 1 805ns 0 1000ns 0)
ex4 vx4 0 vx3 0 1
vx5 vx5 0 pwl (0ns 0 500ns 0 505ns 1 700ns 1 705ns 0 1000ns 0)
ex6 vx6 0 vx5 0 1
*
vy1 vy1 0 pwl (0ns 0 100ns 0 105ns 1 700ns 1 705ns 0 1000ns 0)
ey2 vy2 0 vy1 0 1
vy3 vy3 0 pwl (0ns 0 200ns 0 205ns 1 600ns 1 605ns 0 1000ns 0)
ey4 vy4 0 vy3 0 1
vy5 vy5 0 pwl (0ns 0 300ns 0 305ns 1 500ns 1 505ns 0 1000ns 0)
ey6 vy6 0 vy5 0 1
