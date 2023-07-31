// Encoder Factory
// Binu
// v1.0 05-2017 

#pragma once

#include "EncoderBase.h"
#include "Commondef.h"

namespace Id_Comp
{
	// this class will handle the construction of encoder
	class EncoderFactory
	{
	public:
		EncoderFactory();
		~EncoderFactory();
		static EncoderBase* GetEncoder(CODEC_TYPE codectype_i);
	};
}

