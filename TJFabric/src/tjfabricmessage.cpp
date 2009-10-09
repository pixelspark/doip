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