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
 
 #include "../include/usp3leds.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace usp3;
using namespace tj::ep::leds;

USP3LED::USP3LED(const char* path): _device(path), _r(0.0f), _g(0.0f), _b(0.0f)  {
}

USP3LED::~USP3LED() {
}

void USP3LED::SetColorFading(unsigned char r, unsigned char g, unsigned char b) {	
	_r = r;
	_g = g;
	_b = b;
	UpdateColor(true);
}

void USP3LED::SetColorDirectly(unsigned char r, unsigned char g, unsigned char b) {	
	_r = r;
	_g = g;
	_b = b;
	UpdateColor(false);
}

void USP3LED::UpdateColor(bool fading) {
	_device.WriteRegister(usp3::ChromoflexStripeDevice::KRegisterStatus, 0x01);
	
	// The Chromoflex fades by incrementing the 'level' value with the 'increment' value every 1/100*track seconds,
	// until the level equals the 'set' value. So with an increment of 2 and track of 1, a fade from 0...255 will
	// take 255/2 * 1/100 = 1.27s. The problem is that fades thus do not take equal time (i.e. the amBX driver interpolates
	// a different way, and fades always take 1 second).
	_device.WriteRegisterInt(usp3::ChromoflexStripeDevice::KRegisterIncR, 0x04, 0x04, 0x04, 0x00);
	//_device.WriteRegister(usp3::ChromoflexStripeDevice::KRegisterTrack, 0x01);
	_device.WriteRegisterInt(usp3::ChromoflexStripeDevice::KRegisterSetR, _r, _g, _b, 0x00);
	if(!fading) {
		_device.WriteRegisterInt(usp3::ChromoflexStripeDevice::KRegisterLevelR, _r, _g, _b, 0x00);
	}
}

/** Packet **/
Packet::Packet(): index(0), crc(0xFFFF) {
	Reset();
}

Packet::~Packet() {
}
		
void Packet::Reset() {
	for(unsigned int a=0;a<KMaxPacketLength;a++) {
		data[a] = 0;	
	}
	
	crc = 0x173F;
	index = 0;	
	WriteByteRaw(0xCA);
}

void Packet::WriteByte(unsigned char b) {
	ProcessCRC(b);
	
	if(b == 0xCA) {
		WriteByteRaw(0xCB);
		WriteByteRaw(0x00);	
	}
	else if(b == 0xCB) {
		WriteByteRaw(0xCB);
		WriteByteRaw(0x01);	
	}
	else {
		WriteByteRaw(b);	
	}
}

void Packet::Send(FILE* f) {
	WriteByteRaw((crc & 0xFF00) >> 8);
	WriteByteRaw((crc & 0x00FF));
	fwrite(data, index, 1, f);	
	fflush(f);
	Reset();
}

std::string Packet::ToString() const {
	std::ostringstream wos;
	for(unsigned int i = 0; i < index; i++) {
		wos << std::hex << (int)data[i] << " ";
	}
	return wos.str();
}

void Packet::WriteByteRaw(unsigned char b) {
	if(index > KMaxPacketLength-1) {
		std::cerr << "Packet too long!" << std::endl;
		return;	
	}
	
	data[index] = b;
	++index;	
}

void Packet::ProcessCRC(unsigned char b) {
	crc ^= b;
	
	for(int i = 0; i < 8; i++) {
		if(crc & 1) {
			crc >>= 1;	
			crc ^= 0xA001;
		}	
		else {
			crc >>= 1;	
		}
	}
}

/** ChromoflexStripeDevice **/
ChromoflexStripeDevice::ChromoflexStripeDevice(const char* dev) {
	_fp = fopen(dev, "w");
	if(_fp==0) {
		throw "Could not open device!";	
	}
}	

ChromoflexStripeDevice::~ChromoflexStripeDevice() {
	fclose(_fp);
}

void ChromoflexStripeDevice::WriteRegister(unsigned char address, unsigned char d) {
	Packet p;
	
	// Address
	p.WriteByte(0x00);
	p.WriteByte(0x00);
	p.WriteByte(0x00);
	
	// Data length
	p.WriteByte(0x00);
	p.WriteByte(0x02);	
	
	// Command
	p.WriteByte(0x7E);
	p.WriteByte(address);
	p.WriteByte(d);
	
	p.Send(_fp);
}

void ChromoflexStripeDevice::WriteRegisterInt(unsigned char address, unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
	Packet p;
	
	// Address
	p.WriteByte(0x00);
	p.WriteByte(0x00);
	p.WriteByte(0x00);
	
	// Data length
	p.WriteByte(0x00);
	p.WriteByte(0x05);
	
	// Command
	p.WriteByte(0x7E);
	
	// Address
	p.WriteByte(address);
	
	// Data
	p.WriteByte(a);
	p.WriteByte(b);
	p.WriteByte(c);
	p.WriteByte(d);
	
	p.Send(_fp);	
}

void ChromoflexStripeDevice::WriteReset() {
	Packet p;
	// Address
	p.WriteByte(0x00);
	p.WriteByte(0x00);
	p.WriteByte(0x00);
	
	// Length
	p.WriteByte(0x00);
	p.WriteByte(0x00);
	
	// Command
	p.WriteByte(0xFE);
	
	p.Send(_fp);
}