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

@class MWInputAlertView;

@protocol MWInputDelegate
	- (void)inputAlert:(MWInputAlertView*)alert didClose:(NSString*)value withContext:(NSObject*)context;
@end

@interface MWInputAlertView : UIAlertView <UIAlertViewDelegate> {
	UITextField* _field;
	NSObject* _icontext;
	id<MWInputDelegate> _idelegate;
}

- (id) initWithTitle:(NSString*)title placeholder:(NSString*)placeholder value:(NSString*)value withContext:(NSObject*)context delegate:(id<MWInputDelegate>)d;

@property (nonatomic, retain) UITextField* field;

@end
