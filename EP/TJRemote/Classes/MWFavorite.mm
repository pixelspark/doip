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
 
 #import "MWFavorite.h"

@implementation MWFavorite
@synthesize messagePath = _messagePath;
@synthesize messageArguments = _messageArguments;
@synthesize friendlyName = _friendlyName;
@synthesize specificDevice = _specific;

- (id) initWithPath:(NSString *)path andArguments:(NSMutableArray *)args {
	if(self = [super init]) {
		self.messagePath = path;
		self.messageArguments = args;
	}
	return self;
}

- (void) dealloc {
	[_messagePath release];
	[_messageArguments release];
	[_friendlyName release];
	[super dealloc];
}

- (id) initWithCoder:(NSCoder *)aDecoder {
	if(self = [super init]) {
		self.messagePath = [aDecoder decodeObjectForKey:@"Path"];
		self.messageArguments = [aDecoder decodeObjectForKey:@"Arguments"];
		self.friendlyName = [aDecoder decodeObjectForKey:@"FriendlyName"];
		self.specificDevice = [aDecoder decodeObjectForKey:@"SpecificDevice"];
	}
	return self;
}

- (void) encodeWithCoder:(NSCoder *)aCoder {
	[aCoder encodeObject:self.messagePath forKey:@"Path"];
	[aCoder encodeObject:self.messageArguments forKey:@"Arguments"];
	[aCoder encodeObject:self.friendlyName forKey:@"FriendlyName"];
	[aCoder encodeObject:self.specificDevice forKey:@"SpecificDevice"];
}

@end
