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
 
 #ifndef _TJ_EP_SERVER_MGR_H
#define _TJ_EP_SERVER_MGR_H

#include "epinternal.h"
#include "ependpoint.h"
#include "eppublication.h"
#include <TJNP/include/tjwebserver.h>

namespace tj {
	namespace ep {
		class EPPublication;
		
		/** The EPServerManager class generates a unique 'magic' key for each fabric process. This is
		 used to prevent discovery of services published by the process itself: when the service supports
		 it, services published from this process should have the EPMagicNumber attribute set to the
		 process magic number. When discovering services, the EPMagicNumber attribute should be checked.
		 If it is present, it should be compared with the process magic number. If the magic numbers are
		 equal, the discovery is ignored **/
		class EP_EXPORTED EPServerManager {
			public:
				virtual ~EPServerManager();
				static tj::shared::strong<EPServerManager> Instance();
				virtual tj::shared::ref<tj::np::WebServer> CreateServer(unsigned short port);
				virtual tj::shared::String GetServerMagic();
				
				const static unsigned short KPortDontCare = 0;
				
			protected:
				EPServerManager();
				
				tj::shared::CriticalSection _lock;
				std::map< unsigned short, tj::shared::ref<tj::np::WebServer> > _servers;
				static tj::shared::ref<EPServerManager> _instance;
				tj::shared::String _magic;
		};
		
		class EP_EXPORTED EPStateWebItem: public tj::np::WebItemResource {
			public:
				EPStateWebItem(tj::shared::ref<EPPublication> model, const tj::shared::String& fn);
				virtual ~EPStateWebItem();
				virtual tj::np::Resolution Get(tj::shared::ref<tj::np::WebRequest> frq, std::wstring& error, char** data, tj::shared::Bytes& dataLength);
				
			protected:
				tj::shared::weak<EPPublication> _publication;
		};
		
		/** Serves an XML-file containing the definitions of the messages this fabric will accept **/
		class EP_EXPORTED EPWebItem: public tj::np::WebItemResource {
			public:
				EPWebItem(tj::shared::ref<EPPublication> publication);
				virtual ~EPWebItem();
				virtual tj::shared::ref<tj::np::WebItem> Resolve(const tj::shared::String& file);
				virtual tj::np::Resolution Get(tj::shared::ref<tj::np::WebRequest> frq, std::wstring& error, char** data, tj::shared::Bytes& dataLength);
				virtual tj::shared::String GetDefinitionPath() const;
				virtual tj::shared::String GetStatePath() const;
			
			protected:
				const static wchar_t* KDefinitionPath;
				const static wchar_t* KStatePath;
				tj::shared::ref<EPStateWebItem> _stateItem;
				tj::shared::weak<EPPublication> _publication;
		};
	}
}

#endif