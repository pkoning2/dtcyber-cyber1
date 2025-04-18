//
// Data is a double indexed array of triples x, y, unblank.  First index
// is the character code; second index is the step number (0 to 27).
//
// This data was obtained from a VHDL model of the 6612 synchronizer.
//

typedef struct 
{
    u8 x;
    u8 y;
    u8 unblank;
} xyu;

const xyu dd60chars[060][28] = {
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 1, 2, 1 },
      { 2, 4, 1 },
      { 3, 6, 1 },
      { 3, 6, 1 },
      { 4, 4, 1 },
      { 5, 2, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 5, 2, 1 },
      { 5, 2, 0 },
      { 3, 2, 0 },
      { 1, 2, 1 },
      { 1, 2, 1 },
      { 1, 2, 1 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 4, 1 },
      { 4, 3, 1 },
      { 2, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 2, 3, 1 },
      { 4, 3, 0 },
      { 6, 2, 0 },
      { 6, 1, 1 },
      { 4, 0, 1 },
      { 2, 0, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 2, 0 },
      { 4, 4, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 4, 6, 1 },
      { 2, 6, 1 },
      { 0, 5, 1 },
      { 0, 3, 1 },
      { 0, 1, 1 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 1, 1 },
      { 6, 1, 1 },
      { 6, 1, 1 },
      { 6, 1, 0 },
      { 6, 1, 0 },
      { 6, 1, 0 },
      { 6, 1, 0 },
      { 6, 1, 0 },
      { 6, 1, 0 },
      { 6, 1, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 3, 1 },
      { 6, 1, 1 },
      { 4, 0, 1 },
      { 2, 0, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 4, 4, 1 },
      { 4, 4, 0 },
      { 2, 4, 0 },
      { 0, 4, 1 },
      { 0, 4, 1 },
      { 0, 2, 1 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 4, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 4, 4, 1 },
      { 4, 3, 0 },
      { 4, 3, 0 },
      { 2, 3, 0 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 1, 0 },
      { 4, 3, 0 },
      { 4, 3, 0 },
      { 4, 3, 0 },
      { 6, 3, 1 },
      { 6, 3, 1 },
      { 6, 1, 1 },
      { 4, 0, 1 },
      { 2, 0, 1 },
      { 0, 1, 1 },
      { 0, 3, 1 },
      { 0, 5, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 5, 1 },
      { 6, 5, 1 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 4, 1 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 2, 3, 1 },
      { 4, 3, 1 },
      { 6, 3, 1 },
      { 6, 3, 1 },
      { 6, 1, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 2, 1 },
      { 6, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 2, 1 },
      { 3, 4, 1 },
      { 3, 6, 1 },
      { 3, 6, 1 },
      { 3, 6, 1 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 3, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 2, 1 },
      { 6, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 4, 5, 1 },
      { 2, 4, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 2, 2, 1 },
      { 4, 1, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 4, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 4, 1 },
      { 0, 2, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 4, 1 },
      { 3, 3, 1 },
      { 3, 3, 1 },
      { 5, 5, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 4, 1 },
      { 6, 2, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 4, 1 },
      { 4, 2, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 2, 1 },
      { 6, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 4, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 4, 1 },
      { 6, 2, 1 },
      { 4, 0, 1 },
      { 2, 0, 1 },
      { 0, 2, 1 },
      { 0, 2, 1 },
      { 0, 2, 1 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 4, 1 },
      { 4, 3, 1 },
      { 2, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 2, 0, 0 },
      { 2, 0, 0 },
      { 4, 0, 1 },
      { 6, 2, 1 },
      { 6, 4, 1 },
      { 4, 6, 1 },
      { 2, 6, 1 },
      { 0, 4, 1 },
      { 0, 2, 1 },
      { 2, 0, 1 },
      { 2, 0, 1 },
      { 4, 2, 1 },
      { 4, 2, 0 },
      { 4, 2, 0 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 4, 1 },
      { 4, 3, 1 },
      { 2, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 2, 2, 1 },
      { 4, 1, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 1, 1 },
      { 6, 2, 1 },
      { 4, 3, 1 },
      { 2, 3, 1 },
      { 0, 4, 1 },
      { 0, 5, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 5, 1 },
      { 6, 5, 1 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 2, 1 },
      { 3, 4, 1 },
      { 3, 6, 1 },
      { 3, 6, 1 },
      { 5, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 4, 6, 1 },
      { 2, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 4, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 4, 1 },
      { 0, 2, 1 },
      { 1, 0, 1 },
      { 3, 0, 1 },
      { 5, 0, 1 },
      { 6, 2, 1 },
      { 6, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 4, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 1, 4, 1 },
      { 2, 2, 1 },
      { 3, 0, 1 },
      { 3, 0, 1 },
      { 4, 2, 1 },
      { 5, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 4, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 4, 1 },
      { 0, 2, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 2, 2, 1 },
      { 3, 3, 1 },
      { 3, 3, 1 },
      { 5, 1, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 2, 1 },
      { 6, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 2, 1 },
      { 4, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 4, 1 },
      { 6, 2, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 4, 2, 1 },
      { 2, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 2, 1 },
      { 3, 3, 1 },
      { 3, 3, 1 },
      { 5, 5, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 4, 4, 1 },
      { 3, 3, 1 },
      { 3, 3, 1 },
      { 1, 5, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 4, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 4, 4, 1 },
      { 2, 2, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 3, 1 },
      { 0, 5, 1 },
      { 1, 6, 1 },
      { 3, 6, 1 },
      { 5, 6, 1 },
      { 6, 5, 1 },
      { 6, 3, 1 },
      { 6, 1, 1 },
      { 5, 0, 1 },
      { 3, 0, 1 },
      { 1, 0, 1 },
      { 0, 1, 1 },
      { 0, 1, 1 },
      { 0, 1, 1 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 2, 1 },
      { 3, 4, 1 },
      { 3, 6, 1 },
      { 3, 6, 1 },
      { 1, 5, 1 },
      { 1, 5, 1 },
      { 1, 5, 1 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 4, 0 },
      { 0, 5, 0 },
      { 0, 5, 0 },
      { 0, 5, 0 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 4, 1 },
      { 4, 3, 1 },
      { 2, 2, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 1 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 6, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 1, 1 },
      { 6, 3, 1 },
      { 4, 4, 1 },
      { 2, 4, 1 },
      { 2, 4, 1 },
      { 4, 5, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 4, 6, 1 },
      { 2, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 4, 0, 0 },
      { 4, 0, 0 },
      { 4, 0, 0 },
      { 4, 2, 1 },
      { 4, 4, 1 },
      { 4, 6, 1 },
      { 4, 6, 1 },
      { 2, 4, 1 },
      { 0, 2, 1 },
      { 0, 2, 1 },
      { 2, 2, 1 },
      { 4, 2, 1 },
      { 6, 2, 1 },
      { 6, 2, 1 },
      { 6, 2, 1 },
      { 6, 2, 0 },
      { 6, 2, 0 },
      { 6, 2, 0 },
      { 6, 2, 0 },
      { 6, 2, 0 },
      { 6, 2, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 1, 1 },
      { 6, 3, 1 },
      { 4, 4, 1 },
      { 2, 4, 1 },
      { 0, 4, 1 },
      { 0, 4, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 2, 4, 1 },
      { 4, 4, 1 },
      { 6, 3, 1 },
      { 6, 1, 1 },
      { 4, 0, 1 },
      { 2, 0, 1 },
      { 0, 1, 1 },
      { 0, 3, 1 },
      { 0, 5, 1 },
      { 2, 6, 1 },
      { 4, 6, 1 },
      { 6, 5, 1 },
      { 6, 5, 1 },
      { 6, 5, 1 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 6, 5, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 2, 0, 0 },
      { 2, 0, 0 },
      { 2, 1, 1 },
      { 3, 3, 1 },
      { 5, 5, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 4, 6, 1 },
      { 2, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 1 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 2, 1 },
      { 2, 3, 1 },
      { 4, 3, 1 },
      { 6, 4, 1 },
      { 6, 5, 1 },
      { 4, 6, 1 },
      { 2, 6, 1 },
      { 0, 5, 1 },
      { 0, 4, 1 },
      { 2, 3, 1 },
      { 4, 3, 1 },
      { 6, 2, 1 },
      { 6, 1, 1 },
      { 4, 0, 1 },
      { 2, 0, 1 },
      { 0, 1, 1 },
      { 0, 1, 1 },
      { 0, 1, 1 },
      { 0, 1, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 },
      { 2, 0, 1 },
      { 4, 0, 1 },
      { 6, 1, 1 },
      { 6, 3, 1 },
      { 6, 5, 1 },
      { 4, 6, 1 },
      { 2, 6, 1 },
      { 0, 5, 1 },
      { 0, 3, 1 },
      { 2, 2, 1 },
      { 4, 2, 1 },
      { 6, 3, 1 },
      { 6, 3, 1 },
      { 6, 3, 1 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 0, 0 },
      { 3, 2, 1 },
      { 3, 4, 1 },
      { 3, 6, 1 },
      { 3, 6, 1 },
      { 5, 4, 1 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 4, 3, 1 },
      { 2, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 1 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 0, 3, 0 },
      { 2, 3, 1 },
      { 4, 3, 1 },
      { 6, 3, 1 },
      { 6, 3, 1 },
      { 6, 3, 1 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 6, 3, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 1, 0 },
      { 1, 1, 0 },
      { 1, 1, 0 },
      { 3, 3, 1 },
      { 5, 5, 1 },
      { 5, 5, 1 },
      { 5, 3, 1 },
      { 5, 3, 0 },
      { 3, 3, 0 },
      { 1, 3, 1 },
      { 1, 3, 1 },
      { 1, 5, 1 },
      { 1, 5, 0 },
      { 1, 5, 0 },
      { 3, 3, 1 },
      { 5, 1, 1 },
      { 5, 1, 1 },
      { 5, 1, 1 },
      { 5, 1, 0 },
      { 5, 1, 0 },
      { 5, 1, 0 },
      { 5, 1, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 2, 1 },
      { 4, 4, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 1 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 6, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 4, 0, 0 },
      { 4, 0, 0 },
      { 4, 0, 0 },
      { 3, 1, 1 },
      { 3, 3, 1 },
      { 3, 5, 1 },
      { 4, 6, 1 },
      { 4, 6, 1 },
      { 4, 6, 1 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 4, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 2, 0, 0 },
      { 2, 0, 0 },
      { 2, 0, 0 },
      { 3, 1, 1 },
      { 3, 3, 1 },
      { 3, 5, 1 },
      { 2, 6, 1 },
      { 2, 6, 1 },
      { 2, 6, 1 },
      { 2, 6, 0 },
      { 2, 6, 0 },
      { 2, 6, 0 },
      { 2, 6, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 0, 2, 0 },
      { 2, 2, 1 },
      { 4, 2, 1 },
      { 6, 2, 1 },
      { 6, 2, 1 },
      { 6, 4, 1 },
      { 6, 4, 0 },
      { 6, 4, 0 },
      { 6, 4, 0 },
      { 6, 4, 0 },
      { 6, 4, 0 },
      { 6, 4, 0 },
      { 4, 4, 1 },
      { 2, 4, 1 },
      { 0, 4, 1 },
      { 0, 4, 1 },
      { 0, 4, 1 },
      { 0, 4, 0 },
      { 0, 4, 0 },
      { 0, 4, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 1, 1, 0 },
      { 1, 2, 1 },
      { 1, 2, 1 },
      { 1, 2, 1 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 1, 2, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
    { { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 1 },
      { 0, 0, 1 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } } };

