#import "TTDiscovery.h"

#include <EP/include/epdnssddiscovery.h>
#import "TTMethodMenuItem.h"

TTDiscovery::TTDiscovery() {
	_slt = SocketListenerThread::DefaultInstance();
	_methodMenuItems = nil;
}

ref<Service> TTDiscovery::GetServiceForEndpoint(ref<EPEndpoint> enp) {
	std::map<ref<EPEndpoint>, ref<Service> >::iterator it = _services.find(enp);
	if(it!=_services.end()) {
		return it->second;
	}
	return null;
}

void TTDiscovery::Notify(ref<Object> source, const EPStateChangeNotification& cn) {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	ref<EPRemoteState> epr = cn.remoteState;
	
	if(epr && _methodMenuItems!=nil) {
		// Which endpoint was this for, again?
		ref<EPEndpoint> forEndpoint;
		std::map< ref<EPEndpoint>, ref<EPRemoteState> >::iterator it = _remoteStates.begin();
		while(it!=_remoteStates.end()) {
			if(it->second==epr) {
				forEndpoint = it->first;
				break;
			}
			++it;
		}
		
		if(forEndpoint) {
			for(TTMethodMenuItem* mi in _methodMenuItems) {
				if([mi endpoint]==forEndpoint) {
					[mi update:epr];
				}
			}
		}
	}
	
	[pool release];
}

bool TTDiscovery::GetTagInPreferences(const EPTag& tag, bool& enabled) {
	NSMutableArray* dict = [[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:@"tags"];
	NSString* tagName = [NSString stringWithUTF8String:Mbs(tag).c_str()];
	for (NSMutableDictionary* row in dict) {
		NSString* name = [row valueForKey:@"name"];
		if(name!=nil && [name isEqualToString:tagName]) {
			NSNumber* enabledNr = [row valueForKey:@"enabled"];
			enabled = [enabledNr boolValue];
			return true;
		}
	}
	return false;
}

void TTDiscovery::AddTagToPreferences(const EPTag& tag) {
	NSString* tagName = [NSString stringWithUTF8String:Mbs(tag).c_str()];
	NSMutableArray* dict = [[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKey:@"tags"];
	bool enabled = false;
	if(!GetTagInPreferences(tag,enabled)) {
		NSMutableDictionary* row = [[NSMutableDictionary alloc] init];
		[row setValue:tagName forKey:@"name"];
		[row setValue:[NSNumber numberWithBool:YES] forKey:@"enabled"];
		[dict addObject:row];
		[row release];
	}
	
	[[[NSUserDefaultsController sharedUserDefaultsController] values] setValue:dict forKey:@"tags"];
}

void TTDiscovery::Notify(ref<Object> source, const DiscoveryNotification& dn) {
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	ThreadLock lock(&_lock);
	
	ref<EPEndpoint> enp = dn.endpoint;
	if(dn.added) {
		if(enp && dn.connection) {
			_endpoints.insert(enp);
			_services[enp] = dn.service;
			_connections.insert(std::pair<ref<EPEndpoint>, ref<Connection> >(enp, dn.connection));
			
			std::set<EPTag> tags;
			enp->GetTags(tags);
			
			if(tags.size()==0) {
				AddTagToPreferences(L"");
			}
			else {
				// Add each tag to the set of tags for user defaults
				std::set<EPTag>::const_iterator tit = tags.begin();
				while(tit!=tags.end()) {
					AddTagToPreferences(*tit);
					++tit;
				}
			}
		}
		
		if(enp && dn.remoteState) {
			_remoteStates[enp] = dn.remoteState;
			ref<EPRemoteState>(dn.remoteState)->EventStateChanged.AddListener(this);
		}
	}
	else {
		if(enp) {
			std::set<ref<EPEndpoint> >::iterator eit = _endpoints.find(enp);
			if(eit!=_endpoints.end()) {
				_endpoints.erase(eit);
			}
			
			std::map< ref<EPEndpoint>, ref<Service> >::iterator svcit = _services.find(enp);
			if(svcit!=_services.end()) {
				_services.erase(svcit);
			}
			
			std::map< ref<EPEndpoint>, ref<EPRemoteState> >::iterator sit = _remoteStates.find(enp);
			if(sit!=_remoteStates.end()) {
				_remoteStates.erase(sit);
			}
		}
		else {
			Log::Write(L"TJTrayRemote/TTDiscovery", L"Endpoint leaving, but no endpoint referenced");
		}
		
		std::multimap< ref<EPEndpoint>, ref<Connection> >::iterator it = _connections.begin();
		while(it!=_connections.end()) {
			ref<EPEndpoint> itep = it->first;
			ref<Connection> itcon = it->second;
			if(itep==enp || itcon==dn.connection) {
				_connections.erase(it);
				it = _connections.begin();
			}
			else {
				++it;
			}
		}
	}
	[pool release];
}

void TTDiscovery::OnCreated() {
	ref<EPDiscoveryDefinition> dd = GC::Hold(new EPDiscoveryDefinition());
	_discovery = GC::Hold(new EPDiscovery());
	_discovery->Create(ref<DiscoveryDefinition>(dd), L"");
	_discovery->EventDiscovered.AddListener(this);
}

TTDiscovery::~TTDiscovery() {
	[_methodMenuItems release];
}

