#import <UIKit/UIKit.h>
#import "MWFavorite.h"

#ifdef TARGET_IPAD
@interface MWFavoritesTableViewController : UITableViewController <UIPopoverControllerDelegate, UISplitViewControllerDelegate> {
#else
@interface MWFavoritesTableViewController : UITableViewController {
#endif

	NSMutableArray* _favorites;
	
	#ifdef TARGET_IPAD
		UIPopoverController* _popOverController;
	#endif
}

- (void) addFavorite: (MWFavorite*)fav;
+ (UIImage*) favoriteImage;

@property (nonatomic, retain) NSMutableArray* favorites;

#ifdef TARGET_IPAD
	@property (nonatomic, retain) UIPopoverController* popOverController;
#endif

@end
