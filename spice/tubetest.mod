* testing
.include 3cx100a5.mod
.include 12bh7a.mod
xtest a g 0 3cx100a5
*xtest a g 0 12bh7a
vg g 0 -5v
va a 0 600
.dc va 0 600 10 vg -40 0 5
*.dc va 0 300 10
.end
