#ifndef __APPLE__
enum _bit
{ zero, one } __attribute__((mode (byte)));

typedef enum _bit bit;
#else
// CLANG doesn't support attributes on enums, POS.
typedef unsigned char bit;
#define zero 0
#define one 1
#endif

typedef bit coaxsigs[19];
typedef bit membyte[8];
