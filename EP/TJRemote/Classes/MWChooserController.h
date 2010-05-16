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

@class MWParameter;
@class MWChooserController;

@protocol MWChooserDelegate
- (void) chooser:(MWChooserController*)ch hasChangedParameter:(MWParameter*)param;
@end


@interface MWChooserController : UITableViewController {
	MWParameter* _parameter;
	UITableView* _tableView;
	UIBarButtonItem* _titleLabel;
	id<MWChooserDelegate> _choiceDelegate;
}

- (IBAction) onDone:(id)sender;
- (void) dismiss;
- (void) showInView:(UIViewController*)view;

@property (nonatomic, retain) MWParameter* parameter;
@property (nonatomic, retain) IBOutlet UITableView* tableView;
@property (nonatomic, retain) IBOutlet UIBarButtonItem* titleLabel;
@property (nonatomic, assign) id<MWChooserDelegate> choiceDelegate;

@end
