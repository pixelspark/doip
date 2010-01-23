#import <Cocoa/Cocoa.h>

int main(int argc, char *argv[]) {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    int r = NSApplicationMain(argc,  (const char **) argv);
	[pool release];
	return r;
}
