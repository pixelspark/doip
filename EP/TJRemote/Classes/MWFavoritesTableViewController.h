#import <UIKit/UIKit.h>
#import "MWFavorite.h"

@interface MWFavoritesTableViewController : UITableViewController {
	NSMutableArray* _favorites;
}

- (void) addFavorite: (MWFavorite*)fav;
+ (UIImage*) favoriteImage;

@property (nonatomic, retain) NSMutableArray* favorites;

@end
