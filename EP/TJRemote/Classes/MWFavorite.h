#import <Foundation/Foundation.h>

@interface MWFavorite : NSObject <NSCoding> {
	NSString* _messagePath;
	NSMutableArray* _messageArguments;
	NSString* _friendlyName;
	NSString* _specific;
}

- (id) initWithPath: (NSString*)path andArguments:(NSMutableArray*)args;

@property (nonatomic,retain) NSString* messagePath;
@property (nonatomic,retain) NSMutableArray* messageArguments;
@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, retain) NSString* specificDevice;

@end
