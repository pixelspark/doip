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
 
 #include "../include/eppublication.h"
#include "../include/epservermanager.h"
using namespace tj::shared;
using namespace tj::ep;
using namespace tj::np;
using namespace tj::scout;

EPPublication::EPPublication(strong<EPEndpoint> ep, const std::wstring& magicPostfix): _stateVersion(L"1"), _magicPostfix(magicPostfix) {
	ref<EPEndpoint> epr = ep;
	_ep = epr;
}

void EPPublication::OnCreated() {
	ref<EPEndpoint> ep = _ep;
	if(ep) {
		_ws = EPServerManager::Instance()->CreateServer(EPServerManager::KPortDontCare);
		ref<EPWebItem> resolver = GC::Hold(new EPWebItem(this));
		std::wstring basePath = L"/ep/" + ep->GetFullIdentifier();
		_ws->AddResolver(basePath,ref<WebItem>(resolver));
		
		// Create a list of service attributes
		std::map<std::wstring, std::wstring> attributes;
		attributes[L"EPDefinitionPath"] = basePath + L"/" + resolver->GetDefinitionPath();
		attributes[L"EPStatePath"] = basePath + L"/" + resolver->GetStatePath();
		attributes[L"EPStateVersion"] = _stateVersion;
		attributes[L"EPProtocol"] = L"HTTP";
		
		// Calculate magic number
		std::wstring serverMagic = EPServerManager::Instance()->GetServerMagic();
		if(_magicPostfix.length()>0) {
			serverMagic = serverMagic + L"-" + _magicPostfix;
		}
		attributes[L"EPMagicNumber"] = serverMagic;
		
		// Get the port we're running on and publish the service
		unsigned short actualPort = _ws->GetActualPort();
		_reg = ServiceRegistrationFactory::Instance()->CreateServiceRegistration(ServiceDiscoveryDNSSD, L"_ep._tcp", ep->GetFriendlyName(), actualPort, attributes);
		Log::Write(L"EPFramework/EPPublication", L"EP service active http://localhost:"+Stringify(actualPort)+basePath);
	}
}

ref<EPEndpoint> EPPublication::GetEndpoint() {
	return _ep;
}

void EPPublication::SetState(const std::map< String, Any >& values) {
	ThreadLock lock(&_lock);
	_state = values;
	PublishState();
}

void EPPublication::SetStateVariable(const String& key, const Any& value) {
	ThreadLock lock(&_lock);
	_state[key] = value;
	PublishState();
}

Any EPPublication::GetValue(const String& k) {
	return _state[k];
}

void EPPublication::GetState(EPState::ValueMap& vals) {
	ThreadLock lock(&_lock);
	vals = _state;
}

void EPPublication::LoadState(TiXmlElement* root) {
	ThreadLock lock(&_lock);
	
	TiXmlElement* var = root->FirstChildElement("var");
	while(var!=0) {
		String key = LoadAttributeSmall(var, "key", String(L""));
		Any value;
		value.Load(var);
		_state[key] = value;
		var = var->NextSiblingElement("var");
	}
	
	PublishState();
}

void EPPublication::PublishState() {
	ThreadLock lock(&_lock);
	
	// Recalculate state version; if it is different, then publish
	SecureHash sh;
	std::map< String, Any>::const_iterator it = _state.begin();
	while(it!=_state.end()) {
		sh.AddString(it->first.c_str());
		sh.AddString(L"=");
		sh.AddString(it->second.ToString().c_str());
		sh.AddString(L";");
		++it;
	}
	
	String newVersion = Wcs(sh.GetHashAsString());
	if(_stateVersion!=newVersion) {
		_stateVersion = newVersion;
		_reg->SetAttribute(L"EPStateVersion", _stateVersion);
	}
}

EPPublication::~EPPublication() {
}
