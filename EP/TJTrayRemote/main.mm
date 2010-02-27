#import <Cocoa/Cocoa.h>
#include <TJShared/include/tjshared.h>
using namespace tj::shared;

int main(int argc, char *argv[]) {
	SharedDispatcher sd;
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    int r = NSApplicationMain(argc,  (const char **) argv);
	[pool release];
	return r;
}
