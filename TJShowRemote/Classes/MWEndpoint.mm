#import "MWEndpoint.h"
#include "../../Libraries/TinyXML/tinyxml.h"

@implementation MWEndpoint

@synthesize transportAddress = _transportAddress;

- (id) initWithService:(NSNetService*)service {
	if(self = [super init]) {
		_service = service;
		_methods = [[NSMutableSet alloc] init];
		[_service retain];
		
		// Get properties
		[_service setDelegate:self];
		[_service startMonitoring];
	}
	return self;
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

	NSLog(@"Definition path: %@", path);
	
	// Start fetching the definition file
	NSURL* url = [[NSURL alloc] initWithScheme:@"http" host:[NSString stringWithFormat:@"%@:%d",[_service hostName], [_service port]] path:path];
	NSLog(@"URL is %@", url);
	NSData* xmlData = [NSData dataWithContentsOfURL:url];
	if([xmlData length]>0) {
		TiXmlDocument doc;
		unsigned int xmlDataLength = [xmlData length];
		char* xmlDataString = new char[xmlDataLength+1];
		[xmlData getBytes:(void*)xmlDataString length:xmlDataLength];
		xmlDataString[xmlDataLength] = '\0';
		[xmlData release];
		doc.Parse(xmlDataString, NULL, TIXML_ENCODING_UTF8);
		
		TiXmlElement* root = doc.FirstChildElement("endpoint");
		if(root!=0) {
			int version = 0;
			root->Attribute("version", &version);
			NSLog(@"Endpoint id=%s class=%s friendly=%s version=%d", root->Attribute("id"),root->Attribute("class"), root->Attribute("friendly-name"), version);
			
			// Find a UDP4 transport mechanism that we can use
			TiXmlElement* transports = root->FirstChildElement("transports");
			if(transports!=0) {
				TiXmlElement* transport = transports->FirstChildElement("transport");
				while(transport!=0) {
					if(strcmp(transport->Attribute("type"), "udp")==0) {
						const char* address = transport->Attribute("address");
											
						// If no address is specified, use the host address for this service (when an address is specified, this is multicast)
						if(address==0) {
							self.transportAddress = [service hostName];
						}
						else {
							self.transportAddress = [NSString stringWithUTF8String:address];
						}
						
						transport->Attribute("port", &_transportPort);
						
						NSLog(@"Found transport type=%s address=%s port=%s", transport->Attribute("type"), transport->Attribute("address"), transport->Attribute("port"));
						break;
					}
					
					transport = transport->NextSiblingElement("transport");
				}
			}
			
			TiXmlElement* methods = root->FirstChildElement("methods");
			if(methods!=0) {
				TiXmlElement* method = methods->FirstChildElement("method");
				while(method!=0) {
					NSLog(@"Method id=%s", method->Attribute("id"));
					[_methods addObject:[NSString stringWithUTF8String:method->Attribute("id")]];
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
	[_methods release];
	[super dealloc];
}

@end
