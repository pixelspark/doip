#import "MWEndpoint.h"
#import "MWFavorite.h"
#include "../../../Libraries/TinyXML/tinyxml.h"
#include "../../../Libraries/OSCPack/OscOutboundPacketStream.h"
#include "../../../Libraries/OSCPack/IpEndpointName.h"
#include "../../../Libraries/OSCPack/UdpSocket.h"
#include <arpa/inet.h>

using namespace osc;

@implementation MWEndpoint

@synthesize transportAddress = _transportAddress;
@synthesize methods = _methods;
@synthesize transportFormat = _transportFormat;
@synthesize transportType = _transportType;
@synthesize definitionDownload = _definitionDownload;
@synthesize stateID = _stateID;
@synthesize endpointIdentifier = _id;
@synthesize currentStateURL = _currentStateURL;
@synthesize needStateUpdate = _needStateUpdate;
@synthesize stateDownload = _stateDownload;
@synthesize currentState = _currentState;

- (id) initWithService:(NSNetService*)service {
	if(self = [super init]) {
		_service = service;
		_methods = [[NSMutableArray alloc] init];
		[_service retain];
		
		// Get properties
		[_service setDelegate:self];
		[_service startMonitoring];
		self.currentState = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (NSComparisonResult) compareToEndpoint:(MWEndpoint*)mwe {
	return [[self name] compare:[mwe name]];
}

- (bool) executeFavorite:(MWFavorite *)favorite {
	// Find a method matching the path of the favorite
	for (MWMethod* method in _methods) {
		if([[method pattern] isEqualToString:favorite.messagePath]) {
			// Found; set all the parameters and run it!
			int i =0;
			for(MWParameter* parameter in [method parameters]) {
				MWParameter* savedParameter = [[favorite messageArguments] objectAtIndex:i];
				if(savedParameter==nil) {
					return false; // Not enough parameters
				}
				else {
					if([parameter.identifier isEqualToString:savedParameter.identifier]) {
						parameter.value = savedParameter.value;
					}
				}
				++i;
			}
			[self executeMethod:method];
			return true;
		}
	}
	
	return false;
}

- (void) executeMethod:(MWMethod *)method {
	if(_socket!=0) {
		char buffer[4096];
		OutboundPacketStream ops(buffer,1023);
		ops << osc::BeginMessage([[method pattern] UTF8String]);
		
		// Enumerate parameters and send values
		for (MWParameter* parameter in method.parameters) {
			if([parameter.type isEqualToString:@"int32"]) {
				NSNumber* value = (NSNumber*)parameter.value;
				if(value!=nil) {
					ops << (osc::int32)[value intValue];
				}
			}
			else if([parameter.type isEqualToString:@"double"]) {
				NSNumber* value = (NSNumber*)parameter.value;
				if(value!=nil) {
					ops << (double)[value doubleValue];
				}
			}
			else if([parameter.type isEqualToString:@"bool"]) {
				NSNumber* value = (NSNumber*)parameter.value;
				if(value!=nil) {
					ops << (bool)[value boolValue];
				}
			}
		}
		ops << osc::EndMessage;
		_socket->Send(ops.Data(), ops.Size());
	}
}

- (void) createSocket {
	if(_socket!=0) {
		delete _socket;
		_socket = 0;
	}
	
	try {
		_socket = new UdpSocket();
		if([_transportAddress length]>0) {
			_socket->Connect(IpEndpointName([_transportAddress UTF8String], _transportPort));
		}
		else {
			_socket->Connect(IpEndpointName([[_service hostName] UTF8String], _transportPort));
		}
	}
	catch(const std::exception& e) {
		NSLog(@"Error when creating socket: %s",e.what());
	}
}

- (NSString*) stringFromData:(NSData*) data {
	if(data!=nil) {
		unsigned int dataLen = [data length];
		char* pathBuff = (char*)malloc(dataLen + 1);
		[data getBytes:pathBuff length:dataLen];
		pathBuff[dataLen] = '\0';
		NSString* path = [NSString stringWithUTF8String:pathBuff];
		free(pathBuff);
		return path;
	}
	return nil;
}

- (void) startDownloadingStateAtURL:(NSURL*)url {
	if(self.needStateUpdate) {
		self.needStateUpdate = false;
		if(url!=nil) {
			self.stateDownload = [[MWDownload alloc] initWithURL:url delegate:self];
		}
	}
}

- (void)netService:(NSNetService*)service didUpdateTXTRecordData:(NSData*)data {
	// Parse the properties and get the EP definition file path
	NSDictionary* props = [NSNetService dictionaryFromTXTRecordData:data];
	
	NSString* stateData = [self stringFromData:[props objectForKey:@"EPStateVersion"]];
	if(_stateID==nil || (stateData!=nil && ![_stateID isEqualToString:stateData])) {
		// Need to download state; either now or after definition file download
		self.needStateUpdate = true;
		self.stateID = stateData;
		NSLog(@"Update state (data=%@)", stateData);
	}
	
	if(self.definitionDownload==nil) {
		NSString* path = [self stringFromData:[props objectForKey:@"EPDefinitionPath"]];		
		if(path!=nil) {
			// Start fetching the definition file
			NSURL* url = [[NSURL alloc] initWithScheme:@"http" host:[NSString stringWithFormat:@"%@:%d",[_service hostName], [_service port]] path:path];
			self.definitionDownload = [[MWDownload alloc] initWithURL:url delegate:self];
			[url release];
		}
	}
	
	// Download state; if we can do it now, directly; otherwise, just set the URL
	if(self.needStateUpdate) {
		NSString* statePath = [self stringFromData:[props objectForKey:@"EPStatePath"]];
		if(statePath!=nil) {
			self.currentStateURL = [[NSURL alloc] initWithScheme:@"http" host:[NSString stringWithFormat:@"%@:%d",[_service hostName], [_service port]] path:statePath];
		}
		[self startDownloadingStateAtURL:self.currentStateURL];
	}
	
	[_service startMonitoring];
}

- (NSString*) loadAttribute: (const char*)name fromElement:(TiXmlElement*)element {
	const char* attr = element->Attribute(name);
	if(attr!=0) {
		return [NSString stringWithUTF8String:attr];
	}
	else {
		return nil;
	}
}

- (void) downloadOfDefinition:(MWDownload*)d completed:(NSData*)xmlData {
	TiXmlDocument doc;
	unsigned int xmlDataLength = [xmlData length];
	char* xmlDataString = new char[xmlDataLength+1];
	[xmlData getBytes:(void*)xmlDataString length:xmlDataLength];
	
	xmlDataString[xmlDataLength] = '\0';
	doc.Parse(xmlDataString, NULL, TIXML_ENCODING_UTF8);
	
	
	TiXmlElement* root = doc.FirstChildElement("endpoint");
	if(root!=0) {
		int version = 0;
		root->Attribute("version", &version);
		const char* endpointID = root->Attribute("id");
		if(endpointID!=0) {
			[_id release];
			NSString* epid = [NSString stringWithUTF8String:endpointID];
			const char* nsp = root->Attribute("namespace");
			if(nsp!=0) {
				NSString* nspid = [NSString stringWithUTF8String:nsp];
				_id = [[NSString stringWithFormat:@"%@.%@", nspid, epid] retain];
			}
			else {
				_id = [epid retain];
			}
		}
		
		// Find a UDP4 transport mechanism that we can use
		TiXmlElement* transports = root->FirstChildElement("transports");
		if(transports!=0) {
			TiXmlElement* transport = transports->FirstChildElement("transport");
			while(transport!=0) {
				self.transportType = [self loadAttribute:"type" fromElement:transport];
				self.transportFormat = [self loadAttribute:"format" fromElement:transport];
				
				if([_transportType isEqualToString:@"udp"] && [_transportFormat isEqualToString:@"osc"]) {
					const char* address = transport->Attribute("address");
					
					// If no address is specified, use the host address for this service (when an address is specified, this is multicast)
					if(address==0) {
						self.transportAddress = [self.service hostName];
					}
					else {
						self.transportAddress = [NSString stringWithUTF8String:address];
					}
					
					transport->Attribute("port", &_transportPort);
					if(_transportPort!=0) {
						[self createSocket];
						break;
					}
				}
				
				transport = transport->NextSiblingElement("transport");
			}
		}
		
		TiXmlElement* methods = root->FirstChildElement("methods");
		if(methods!=0) {
			TiXmlElement* method = methods->FirstChildElement("method");
			while(method!=0) {
				TiXmlElement* firstPattern = method->FirstChildElement("path");
				
				if(firstPattern!=0) {
					TiXmlNode* firstPatternText = firstPattern->FirstChild();
					
					if(firstPatternText!=0) {
						MWMethod* mt = [[MWMethod alloc] initWithPattern:[NSString stringWithUTF8String:firstPatternText->Value()] friendlyName:[NSString stringWithUTF8String:method->Attribute("friendly-name")] endpoint:self];
						[_methods addObject:mt];
						
						// Load description
						TiXmlElement* description = method->FirstChildElement("description");
						if(description!=0) {
							TiXmlNode* descriptionText = description->FirstChild();
							if(descriptionText!=0) {
								mt.friendlyDescription = [NSString stringWithUTF8String:descriptionText->Value()];
							}
						}
						
						// Load parameters
						TiXmlElement* firstParameter = method->FirstChildElement("parameter");
						while(firstParameter!=0) {
							MWParameter* param = [[MWParameter alloc] initFromDefinition:firstParameter inMethod:mt];
							[mt.parameters addObject:param];
							[param release];
							firstParameter = firstParameter->NextSiblingElement("parameter");
						}
						
						[mt release];
					}
				}
				method = method->NextSiblingElement("method");
			}
		} 
		
	}
	
	delete[] xmlDataString;
	
	// Start updating state if we still have to
	if(self.needStateUpdate && self.currentStateURL!=nil) {
		[self startDownloadingStateAtURL:self.currentStateURL];
	}
}

- (NSString*) attribute:(const char*)name ofElement:(TiXmlElement*)el {
	if(el!=NULL) {
		const char* value = el->Attribute(name);
		if(value!=NULL) {
			return [NSString stringWithUTF8String:value];
		}
	}
	return nil;
}

- (void) downloadOfState:(MWDownload*)d completed:(NSData*)xmlData {
	NSLog(@"State data download completed");
	
	TiXmlDocument doc;
	unsigned int xmlDataLength = [xmlData length];
	char* xmlDataString = new char[xmlDataLength+1];
	[xmlData getBytes:(void*)xmlDataString length:xmlDataLength];
	
	xmlDataString[xmlDataLength] = '\0';
	doc.Parse(xmlDataString, NULL, TIXML_ENCODING_UTF8);
	TiXmlElement* root = doc.FirstChildElement("state");
	
	if(root!=NULL) {
		TiXmlElement* var = root->FirstChildElement("var");
		while(var!=NULL) {
			NSString* key = [self attribute:"key" ofElement:var];
			NSString* value = [self attribute:"value" ofElement:var];
			NSString* type = [self attribute:"type" ofElement:var];
			
			if(key!=nil && value!=nil && type!=nil) {
				NSString* existingValue = [self.currentState objectForKey:key];
				bool changed = true;
				if(existingValue!=nil && [existingValue isEqualToString:value]) {
					changed = false;
				}
				[self.currentState setObject:value forKey:key];
				if(changed) {
					MWStateChange* info = [[MWStateChange alloc] initWithEndpoint:self key:key value:value];
					NSNotification* nt = [NSNotification notificationWithName:@"MWStateChange" object:info];
					[[NSNotificationCenter defaultCenter] postNotification:nt];
					[info release];
				}
			}
			
			var = var->NextSiblingElement("var");
		}
	}
	
	delete[] xmlDataString;
	
	self.stateDownload = nil;
	self.currentStateURL = nil;
}

- (void) download:(MWDownload*)d completed:(NSData*)xmlData {
	if(xmlData!=nil && [xmlData length]>0) {
		if(d==_definitionDownload) {
			[self downloadOfDefinition:d completed:xmlData];
		}
		else if(d==_stateDownload) {
			[self downloadOfState:d completed:xmlData];
		}
	}
}

- (NSNetService*) service {
	return _service;
}

- (NSString*) name {
	return [_service name];
}

- (void) dealloc {
	[_id release];
	[_service stopMonitoring];
	[_service release];
	[_transportAddress release];
	[_transportFormat release];
	[_transportType release];
	[_methods release];
	[_definitionDownload release];
	[_stateDownload release];
	[_currentStateURL release];
	self.currentState = nil;
	delete _socket;
	[super dealloc];
}

@end

@implementation MWStateChange

@synthesize endpoint = _endpoint;
@synthesize key = _key;
@synthesize value = _value;

- (id) initWithEndpoint:(MWEndpoint *)ep key:(NSString *)k value:(NSString *)v {
	if(self = [super init]) {
		self.endpoint = ep;
		self.key = k;
		self.value = v;
	}
	return self;
}

- (void) dealloc {
	self.endpoint = nil;
	self.key = nil;
	self.value = nil;
	[super dealloc];
}

@end

