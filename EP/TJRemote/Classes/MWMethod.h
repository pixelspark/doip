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
 
 #ifndef MW_METHOD_H
#define MW_METHOD_H
#import <UIKit/UIKit.h>
#import "MWParameter.h"

@class MWEndpoint;
@class MWMethod;
@class MWFavorite;
@class MWSliderView;
class TiXmlElement;

@interface MWMethod : NSObject {
	NSString* _pattern;
	NSString* _friendly;
	NSString* _description;
	NSString* _bindEnabled;
	bool _enabled;
	NSMutableArray* _parameters;
	MWEndpoint* _parent;
}

@property (nonatomic, retain) NSString* pattern;
@property (nonatomic, retain) NSString* bindEnabled;
@property (nonatomic, retain) NSMutableArray* parameters;
@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, retain) NSString* friendlyDescription;
@property (nonatomic, assign) MWEndpoint* parent;

- (id) initWithPattern: (NSString*)pattern friendlyName:(NSString*)fn endpoint:(MWEndpoint*)e bindEnabledTo:(NSString*)bindEnabled ;
- (void) setupCell: (UITableViewCell*)cell inController:(UIViewController*)cs;
- (bool) parametersFitInCell;
- (void) execute;
- (MWFavorite*) createFavorite;
- (bool) enabled;

@end

#endif
