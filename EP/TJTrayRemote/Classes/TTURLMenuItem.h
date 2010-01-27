#import <Cocoa/Cocoa.h>

@interface TTURLMenuItem : NSMenuItem {
	NSString* _url;
}

- (id) initWithTitle:(NSString*)title url:(NSString*)url;

@end
