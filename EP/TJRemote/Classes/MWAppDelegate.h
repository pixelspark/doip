/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 #import <UIKit/UIKit.h>
#import "MWClient.h"

@class MWChooserController;
@class MWEndpointsTableViewController;
@class MWFavoritesTableViewController;

@interface MWAppDelegate : NSObject <UIApplicationDelegate, MWClientDelegate> {
    UIWindow *window;
	MWEndpointsTableViewController* endpointsController;
	UINavigationController* endpointsNavigationController;
	
	#ifdef TARGET_IPAD
		id<UISplitViewControllerDelegate> splitViewDelegate;
		UISplitViewController* splitViewController;
	#endif
	
	bool startingUp;
	MWChooserController* chooserController;
	MWFavoritesTableViewController* favoritesController;
	
	// Splash screen items
	UIView* regularBackground;
	UIView* splashBackground;
	UIView* splashThrobber;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UIView* regularBackground;
@property (nonatomic, retain) IBOutlet UIView* splashBackground;
@property (nonatomic, retain) IBOutlet UIView* splashThrobber;
@property (nonatomic, retain) IBOutlet MWEndpointsTableViewController* endpointsController;
@property (nonatomic, retain) IBOutlet UINavigationController* endpointsNavigationController;
@property (nonatomic, retain) IBOutlet MWChooserController* chooserController;
@property (nonatomic, retain) IBOutlet MWFavoritesTableViewController* favoritesController;

#ifdef TARGET_IPAD
	@property (nonatomic, retain) IBOutlet UISplitViewController* splitViewController;
	@property (nonatomic, retain) IBOutlet id<UISplitViewControllerDelegate> splitViewDelegate;
#endif

@end

