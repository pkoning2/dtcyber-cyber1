* testing
.include 3cx100a5.mod
xtest a g 0 3cx100a5
vg g 0 0v
va a 0 1000v
*.dc va 0 1200 10 vg -15 +30 1
.dc va 0 1200 10
.end
