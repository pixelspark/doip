#ifndef _USP3_H
#define _USP3_H

#include <string>
#include <stdio.h>

namespace usp3 {
	class Packet {
		public:
			Packet();
			virtual ~Packet();
			void Reset();
			void WriteByte(unsigned char b);		
			void Send(FILE* f);	
			std::string ToString() const;	
			
		protected:	
			void WriteByteRaw(unsigned char b);		
			void ProcessCRC(unsigned char b);
				
			const static unsigned int KMaxPacketLength = 64;
			unsigned char data[KMaxPacketLength];
			unsigned int index;
			unsigned short crc;
	};
	
	class ChromoflexStripeDevice {
		public:
			ChromoflexStripeDevice(const char* dev);		
			virtual ~ChromoflexStripeDevice();		
			void WriteRegister(unsigned char address, unsigned char d);		
			void WriteRegisterInt(unsigned char address, unsigned char a, unsigned char b, unsigned char c, unsigned char d);		
			void WriteReset();
					
			const static unsigned char KRegisterStatus = 18;
			const static unsigned char KRegisterTrack = 17;
			const static unsigned char KRegisterSetR = 4;
			const static unsigned char KRegisterSetG = KRegisterSetR+1;
			const static unsigned char KRegisterSetB = KRegisterSetR+2;
			const static unsigned char KRegisterSetX = KRegisterSetR+3;
			
			const static unsigned char KRegisterLevelR = 0;
			const static unsigned char KRegisterLevelG = KRegisterLevelR+1;
			const static unsigned char KRegisterLevelB = KRegisterLevelR+2;
			const static unsigned char KRegisterLevelX = KRegisterLevelR+3;
			
			const static unsigned char KRegisterIncR = 8;
			const static unsigned char KRegisterIncG = KRegisterIncR+1;
			const static unsigned char KRegisterIncB = KRegisterIncR+2;
			const static unsigned char KRegisterIncX = KRegisterIncR+3;
			
			const static unsigned char KRegisterProgramHigh = 21;
			const static unsigned char KRegisterProgramLow = 22;
			
		protected:
			FILE* _fp;
	};
}

#endif