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
 
 /* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJ_EP_EPENDPOINTSERVER_H
#define _TJ_EP_EPENDPOINTSERVER_H

#include "epinternal.h"
#include "ependpoint.h"
#include "epconnection.h"
#include "eppublication.h"

#pragma warning (push)
#pragma warning (disable: 4251 4275)

namespace tj {
	namespace ep {
		template<class T> class EPEndpointServer: public EPEndpoint, public tj::shared::Listener<MessageNotification> {
			public:
				typedef void (T::*Member)(tj::shared::strong<Message>, tj::shared::ref<Connection> connection, tj::shared::ref<ConnectionChannel> channel);
				
				EPEndpointServer(const tj::shared::String& id, const tj::shared::String& nameSp, const tj::shared::String& friendlyName);
				virtual ~EPEndpointServer();
				virtual void AddMethod(tj::shared::strong<EPMethod> method, Member m);
				virtual void AddTransport(tj::shared::strong<EPTransport> ept, tj::shared::ref<Connection> epc);
				virtual void BindVariable(const tj::shared::String& key, tj::shared::Any* value);
				virtual void Notify(tj::shared::ref<tj::shared::Object> src, const MessageNotification& data);
				virtual tj::shared::String GetID() const;
				virtual tj::shared::String GetNamespace() const;
				virtual tj::shared::String GetFriendlyName() const;
				virtual tj::shared::String GetVersion() const;
				virtual bool IsDynamic() const;
				virtual bool IsPublished() const;
				virtual void Publish(bool t);
				virtual void Publish(tj::shared::ref<EPPublication> ep);
				virtual void GetMethods(std::vector< tj::shared::ref<EPMethod> >& methodList) const;
				virtual void GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const;
				virtual void RemoveAllMethods();
				virtual void GetTags(std::set<EPTag>& tags) const;
				virtual void SetTags(const tj::shared::String& tagList);
			
			protected:
				virtual void UpdateStatePublication();
			
				typedef typename std::map<tj::shared::ref<EPMethod>, Member> MemberMap;
				typedef typename std::map<tj::shared::ref<EPTransport>, tj::shared::ref<Connection> > TransportMap;
				typedef typename std::map<tj::shared::String, tj::shared::Any* > StateMap;
			
				MemberMap _members;
				TransportMap _transports;
				StateMap _variables;
			
				tj::shared::String _id;
				tj::shared::String _ns;
				tj::shared::String _friendlyName;
				bool _updateDefaultValuesToState;
				tj::shared::ref<EPPublication> _publication;
				std::set< EPTag > _tags;
				mutable tj::shared::CriticalSection _lock;
		};
		
		template<class T> EPEndpointServer<T>::EPEndpointServer(const tj::shared::String& id, const tj::shared::String& ns, const tj::shared::String& friendlyName): _id(id), _friendlyName(friendlyName), _ns(ns), _updateDefaultValuesToState(true) {
		}
		
		template<class T> EPEndpointServer<T>::~EPEndpointServer() {
		}
		
		template<class T> tj::shared::String EPEndpointServer<T>::GetID() const {
			return _id;
		}
		
		template<class T> tj::shared::String EPEndpointServer<T>::GetNamespace() const {
			return _ns;
		}
		
		template<class T> tj::shared::String EPEndpointServer<T>::GetFriendlyName() const {
			return _friendlyName;
		}
		
		template<class T> tj::shared::String EPEndpointServer<T>::GetVersion() const {
			return L"";
		}
		
		template<class T> bool EPEndpointServer<T>::IsDynamic() const {
			return true;
		}
		
		template<class T> void EPEndpointServer<T>::AddMethod(tj::shared::strong<EPMethod> epm, Member m) {
			tj::shared::ThreadLock lock(&_lock);
			_members[tj::shared::ref<EPMethod>(epm)] = m;
		}
		
		template<class T> void EPEndpointServer<T>::AddTransport(tj::shared::strong<EPTransport> ept, tj::shared::ref<Connection> epc) {
			tj::shared::ThreadLock lock(&_lock);
			_transports[ept] = epc;
		}
		
		template<class T> void EPEndpointServer<T>::BindVariable(const tj::shared::String& key, tj::shared::Any* value) {
			tj::shared::ThreadLock lock(&_lock);
			_variables[key] = value;
			UpdateStatePublication();
		}

		template<class T> void EPEndpointServer<T>::RemoveAllMethods() {
			_members.clear();
		}
		
		template<class T> bool EPEndpointServer<T>::IsPublished() const {
			return _publication;
		}
		
		template<class T> void EPEndpointServer<T>::Publish(bool t) {
			tj::shared::ThreadLock lock(&_lock);
			if(t) {
				_publication = tj::shared::GC::Hold(new EPPublication(tj::shared::ref<EPEndpoint>(this)));
				UpdateStatePublication();
			}
			else {
				_publication = tj::shared::null;
			}
		}
		
		template<class T> void EPEndpointServer<T>::Publish(tj::shared::ref<EPPublication> pub) {
			tj::shared::ThreadLock lock(&_lock);
			_publication = pub;
			UpdateStatePublication();
		}
		
		template<class T> void EPEndpointServer<T>::GetTags(std::set<EPTag>& tags) const {
			tj::shared::ThreadLock lock(&_lock);
			tags = _tags;
		}
		
		template<class T> void EPEndpointServer<T>::SetTags(const tj::shared::String& tagList) {
			std::vector< tj::shared::String > parts = tj::shared::Explode<tj::shared::String>(tagList, L" ");
			
			tj::shared::ThreadLock lock(&_lock);
			_tags.clear();
			std::vector< tj::shared::String >::const_iterator it = parts.begin();
			while(it!=parts.end()) {
				_tags.insert(*it);
				++it;
			}
		}
		
		template<class T> void EPEndpointServer<T>::GetMethods(std::vector< tj::shared::ref<EPMethod> >& methodList) const {
			tj::shared::ThreadLock lock(&_lock);
			typename MemberMap::const_iterator it = _members.begin();
			while(it!=_members.end()) {
				tj::shared::ref<EPMethod> epm = it->first;
				if(epm) {
					methodList.push_back(epm);
				}
				++it;
			}
		}
		
		template<class T> void EPEndpointServer<T>::GetTransports(std::vector< tj::shared::ref<EPTransport> >& transportsList) const {
			tj::shared::ThreadLock lock(&_lock);
			typename TransportMap::const_iterator it = _transports.begin();
			while(it!=_transports.end()) {
				tj::shared::ref<EPTransport> ept = it->first;
				if(ept) {
					transportsList.push_back(ept);
				}
				++it;
			}
		}
		
		template<class T> void EPEndpointServer<T>::Notify(tj::shared::ref<tj::shared::Object> src, const MessageNotification& data) {
			tj::shared::ThreadLock lock(&_lock);
			tj::shared::ref<Message> m = data.message;
			if(m) {
				tj::shared::String path = m->GetPath();
				typename MemberMap::iterator it = _members.begin();
				while(it!=_members.end()) {
					tj::shared::ref<EPMethod> epm = it->first;
					if(epm && epm->Matches(m)) {
						epm->PersistDefaultValues(m);						
						
						// Execute the associated handler
						Member mem = it->second;
						if(mem!=0) {
							(static_cast<T*>(this)->*mem)(data.message, data.source, data.channel);
						}
						
						// Update state variables that are published
						UpdateStatePublication();

						return;
					}
					++it;
				}
			}
		}
		
		template<class T> void EPEndpointServer<T>::UpdateStatePublication() {
			tj::shared::ThreadLock lock(&_lock);
			if(_publication) {
				std::map< tj::shared::String, tj::shared::Any > values;
				StateMap::const_iterator it = _variables.begin();
				while(it!=_variables.end()) {
					values[it->first] = *(it->second);
					++it;
				}
				_publication->SetState(values);
			}
		}
	}
}

#endif
