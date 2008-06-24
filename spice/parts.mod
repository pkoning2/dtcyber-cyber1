* Component models for active devices
.model mz56 d(bv=5.6V)
.model mz12 d(bv=12V)
.model mz16 d(bv=16V)
*
* 2N2405 (as a guess for something close)
.model mnpn npn(level=1, bf=60)
* 2N2303 (as a guess)
.model mpnp pnp(level=1, bf=75)
*
* Tubes
.include 12bh7a.mod
.include 3cx100a5.mod
