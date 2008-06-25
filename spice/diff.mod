* Differential signal generator
*
* 0..1 volt input to lo..hi volt differential pair
*
* lo and hi would be parameters if that worked, but it doesn't so 
* so they are hardcoded to the 6000 series logic levels.
*
* Logic levels in 6600 modules
*.param lo=1.2v
*.param hi=0.2v
*
.subckt diff in out1 out2
b1 out1 0 v=1.2 - v(in)
b2 out2 0 v=0.2 + v(in)
.ends diff
