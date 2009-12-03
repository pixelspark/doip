#import <UIKit/UIKit.h>
#import "MWFavorite.h"

@interface MWFavoritesTableViewController : UITableViewController {
	NSMutableArray* _favorites;
}

- (void) addFavorite: (MWFavorite*)fav;

@property (nonatomic, retain) NSMutableArray* favorites;

@end
