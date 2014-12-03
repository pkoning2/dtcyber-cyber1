#import <AppKit/NSScreen.h>

float mainDisplayScale (void)
{
    return [[NSScreen mainScreen] backingScaleFactor];
}
