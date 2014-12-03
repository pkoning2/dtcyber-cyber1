#import <AppKit/NSScreen.h>

float mainDisplayScale ()
{
    return [[NSScreen mainScreen] backingScaleFactor];
}
