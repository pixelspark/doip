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
#import "../../../Libraries/TinyXML/tinyxml.h"

@class MWMethod;
class TiXmlElement;

@interface MWParameter: NSObject <UITextFieldDelegate, NSCoding> {
	NSString* _friendly;
	NSString* _id;
	NSString* _type;
	NSString* _min;
	NSString* _max;
	NSString* _default;
	NSString* _value;
	NSString* _nature;
	MWMethod* _parent;
	NSString* _bindValue;
	NSMutableDictionary* _options;
}

- (id) initFromDefinition:(TiXmlElement*)def inMethod:(MWMethod*)m;
- (UIView*) createView:(CGRect)rect immediate:(BOOL)imm inController:(UIViewController*)c;
- (void) textValueChanged: (UIView*)view event:(UIEvent*)evt;
- (void) sliderValueChanged: (UIView*)slider event:(UIEvent*)evt;
- (void) executeHandler: (UIView*)vw event:(UIEvent*)evt;
- (void) switchValueChanged: (UISwitch*)sw event:(UIEvent*)evt;
- (MWParameter*) clone;

- (id) initWithCoder:(NSCoder *)aDecoder;
- (void) encodeWithCoder:(NSCoder *)aCoder;

@property (nonatomic, retain) NSString* friendlyName;
@property (nonatomic, retain) NSString* minimumValue;
@property (nonatomic, retain) NSString* maximumValue;
@property (nonatomic, retain) NSString* defaultValue;
@property (nonatomic, retain) NSString* type;
@property (nonatomic, assign) MWMethod* parent;
@property (nonatomic, retain) NSString* value;
@property (nonatomic, retain) NSString* identifier;
@property (nonatomic, readonly) bool discrete;
@property (nonatomic, retain) NSString* nature;
@property (nonatomic, retain) NSDictionary* options;
@property (nonatomic, retain) NSString* bindValue;

@end
