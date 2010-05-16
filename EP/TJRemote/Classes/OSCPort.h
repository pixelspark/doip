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
 
 //
//  OSCPort.h
//  ObjCOSC
//
//  Created by C. Ramakrishnan on Tue Oct 01 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "../OSCKit/OSC-client.h"

@interface OSCPort : NSObject {
    int	    _sock;
    char*   _types;
    OSCbuf* _oscBuffer;
	void*   _oscDataBuffer;
	BOOL    _ownsSocket;
}

#pragma mark _____ instance creation
// ipAddress is a character string representing a numeric address in the standard
// Internet . notation. *I don't do name lookup*
+ (id)oscPortToAddress:(const char*)ipAddress portNumber:(unsigned short)portNumber;
+ (id)oscPortWithAddress:(struct sockaddr*)address;

// socket needs to be a socket connected to the server
- (id)initWithSocket:(int)socket;

#pragma mark _____ accessing
- (int)socket;
- (BOOL)isSocketOwner;
- (void)setOwnsSocket:(BOOL)ownsSocket;

#pragma mark _____ actions
// If you use a single OSCPort in multiple threads, you need
// an NSLock/mutex around calls to these actions
- (BOOL)loadSynthDef:(char*)synthDefFilename;
- (BOOL)newSynthFromDef:(char*)synthDefName 
                synthID:(int)synthID 
            parentGroup:(int)parentGroup;
- (BOOL)freeSynth:(int)synthID;

- (BOOL)sendTo:(char*)address types:(char*)types, ...;

// build up a message progressively -- works when you
// know the signature beforehand, but don't know exactly
// what the values will be
- (void)beginSendTo:(char*)address types:(char*)types;
- (void)appendInt:(int)value;
- (void)appendFloat:(float)value;
- (void)appendString:(char*)value;
- (BOOL)completeSend;


@end
