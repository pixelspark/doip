#import "MWEndpoint.h"
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
@synthesize download = _download;

- (id) initWithService:(NSNetService*)service {
	if(self = [super init]) {
		_service = service;
		_methods = [[NSMutableArray alloc] init];
		[_service retain];
		
		// Get properties
		[_service setDelegate:self];
		[_service startMonitoring];
	}
	return self;
}

- (void) executeMethod:(MWMethod *)method {
	if(_socket!=0) {
		char buffer[4096];
		OutboundPacketStream ops(buffer,1023);
		ops << osc::BeginMessage([[method pattern] UTF8String]);
		
		// Enumerate parameters and send values
		for (MWParameter* parameter in method.parameters) {
			NSLog(@"Parameter");
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

- (void)netService:(NSNetService*)service didUpdateTXTRecordData:(NSData*)data {
	// Parse the properties and get the EP definition file path
	[_service stopMonitoring];
	NSDictionary* props = [NSNetService dictionaryFromTXTRecordData:data];
	NSData* dpData = [props objectForKey:@"EPDefinitionPath"];
	unsigned int dataLen = [dpData length];
	char* pathBuff = (char*)malloc(dataLen + 1);
	[dpData getBytes:pathBuff length:dataLen];
	pathBuff[dataLen] = '\0';
	NSString* path = [NSString stringWithUTF8String:pathBuff];
	free(pathBuff);
	
	// Start fetching the definition file
	NSURL* url = [[NSURL alloc] initWithScheme:@"http" host:[NSString stringWithFormat:@"%@:%d",[_service hostName], [_service port]] path:path];
	self.download = [[MWDownload alloc] initWithURL:url delegate:self];
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

- (void) download:(MWDownload*)d completed:(NSData*)xmlData {
	if(xmlData!=nil && [xmlData length]>0) {
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
						[self createSocket];
						break;
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
	}
	
}

- (NSNetService*) service {
	return _service;
}

- (NSString*) name {
	return [_service name];
}

- (void) dealloc {
	[_service stopMonitoring];
	[_service release];
	[_transportAddress release];
	[_transportFormat release];
	[_transportType release];
	[_methods release];
	[_download release];
	delete _socket;
	[super dealloc];
}

@end
