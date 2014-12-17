#if defined (__ppc__) || defined (__ppc64__)

float mainDisplayScale (void)
{
    return 1.0;
}
#else
#import <AppKit/NSScreen.h>

float mainDisplayScale (void)
{
    return [[NSScreen mainScreen] backingScaleFactor];
}
#endif
