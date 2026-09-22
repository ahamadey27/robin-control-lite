#import <Cocoa/Cocoa.h>

// A console executable linking plugin shared code has no host NSApplication.
// Provide one so the real JUCE async controller can run its message loop.
void initialiseLicensingTestApplication()
{
    [NSApplication sharedApplication];
}
