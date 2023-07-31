// Decoder Factory
// Binu
// v1.0 05-2017 

#pragma once

#include "DecoderBase.h"
#include "Commondef.h"

namespace Id_Comp
{
	// this class will handle the construction of decoder
	class DecoderFactory
	{
	public:
		DecoderFactory();
		~DecoderFactory();
		static DecoderBase* GetDecoder(CODEC_TYPE codectype_i);
	};
}

