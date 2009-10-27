#include "tjoscutil.h"
#include <limits>
using namespace tj::shared;
using namespace tj::fabric::connections;
using namespace osc;

bool OSCUtil::ArgumentToAny(const osc::ReceivedMessageArgument& arg, tj::shared::Any& any) {
	const char type = arg.TypeTag();
	switch(type) {
		case osc::TRUE_TYPE_TAG:
			any = Any(true);
			return true;
			
		case osc::FALSE_TYPE_TAG:
			any = Any(false);
			return true;
			
		case osc::NIL_TYPE_TAG:
			any = Any();
			return true;
			
		case osc::INFINITUM_TYPE_TAG:
			any = Any(std::numeric_limits<double>::infinity());
			return true;
			
		case osc::INT32_TYPE_TAG:
			any = Any((int)arg.AsInt32());
			return true;
			
		case osc::FLOAT_TYPE_TAG:
			any = Any((float)arg.AsFloat());
			return true;
			
		case osc::CHAR_TYPE_TAG:
			any = Any(Stringify(arg.AsChar()));
			return true;
			
		case osc::RGBA_COLOR_TYPE_TAG: {
			strong<Tuple> data = GC::Hold(new Tuple(4));
			unsigned int color = arg.AsRgbaColor();
			
			data->Set(0, Any(double(int((color >> 24) & 0xFF)) / 255.0));
			data->Set(1, Any(double(int((color >> 16) & 0xFF)) / 255.0));
			data->Set(2, Any(double(int((color >> 8) & 0xFF)) / 255.0));
			data->Set(3, Any(double(int(color & 0xFF)) / 255.0));
			any = Any(data);
			return true;								
		}
			
		case osc::MIDI_MESSAGE_TYPE_TAG: {
			strong<Tuple> data = GC::Hold(new Tuple(4));
			unsigned int msg = arg.AsMidiMessage();
			
			data->Set(0, Any(double(int((msg >> 24) & 0xFF)) / 255.0));
			data->Set(1, Any(double(int((msg >> 16) & 0xFF)) / 255.0));
			data->Set(2, Any(double(int((msg >> 8) & 0xFF)) / 255.0));
			data->Set(3, Any(double(int(msg & 0xFF)) / 255.0));
			any = Any(data);
			return true;											  
		}
			
		case osc::DOUBLE_TYPE_TAG:
			any = Any(arg.AsDouble());
			return true;
			
		case osc::STRING_TYPE_TAG:
			any = Any(Wcs(std::string(arg.AsString())));
			return true;
			
		case osc::SYMBOL_TYPE_TAG:
			any = Any(Wcs(std::string(arg.AsSymbol())));
			return true;
			
		case osc::BLOB_TYPE_TAG:
			// TODO: implement this in some way... We cannot convert blob data to a string,
			// since it can contain \0 characters. It is thus not very suited for Any...
			
		case osc::INT64_TYPE_TAG:
			// TODO: add Any::TypeInt64 or something like that...
			
		case osc::TIME_TAG_TYPE_TAG:
			// TODO: conversion to Time type (and addition of Any::TypeTime?)
			
		default:
			return false;
	};
}