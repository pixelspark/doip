#include "../include/tjfabricmessage.h"
using namespace tj::shared;
using namespace tj::fabric;

Message::Message(const tj::shared::String& path): _path(path) {
}

Message::~Message() {
}

const tj::shared::String& Message::GetPath() const {
	return _path;
}

String Message::GetParameterTags() const {
	std::wostringstream wos;
	std::map<unsigned int, Any>::const_iterator it = _parameters.begin();
	while(it!=_parameters.end()) {
		const Any& val = it->second;
		switch(val.GetType()) {
			case Any::TypeBool: {
				if((bool)val) {
					wos << L'T';
				}
				else {
					wos << L'F';
				}
				break;
			}
		
			case Any::TypeDouble:
				wos << L'd';
				break;
				
			case Any::TypeInteger:
				wos << L'i';
				break;
				
			case Any::TypeNull:
				wos << L'N';
				break;
				
			case Any::TypeObject:
				wos << L'b';
				break;
				
			case Any::TypeString:
				wos << L's';
				break;
				
			default:
			case Any::TypeTuple:
				wos << L'?';
				break;
		}
		++it;
	}
	return wos.str();
}

void Message::SetPath(const tj::shared::String& m) {
	_path = m;
}

void Message::SetParameter(unsigned int i, const Any& d) {
	_parameters[i] = d;
}

unsigned int Message::GetParameterCount() const {
	return _parameters.size();
}

Any Message::GetParameter(unsigned int i) const {
	std::map<unsigned int, Any>::const_iterator it = _parameters.find(i);
	if(it!=_parameters.end()) {
		return it->second;
	}

	Throw(L"Invalid parameter index: ", ExceptionTypeError);
}

tj::shared::String Message::ToString() const {
	std::wostringstream wos;
	wos << _path << L" ";
	std::map< unsigned int, Any >::const_iterator it = _parameters.begin();
	while(it!=_parameters.end()) {
		wos << it->first << L":" << it->second.ToString() << L" ";
		++it;
	}
	
	return wos.str();
}