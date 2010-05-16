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
 
 #include "../include/epmessage.h"
using namespace tj::shared;
using namespace tj::ep;

Message::Message(const tj::shared::String& path): _path(path) {
}

Message::~Message() {
}

void Message::OnRecycle() {
	_parameters.clear();
	_path = L"";
}

void Message::OnReuse() {
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