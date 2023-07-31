// Decoder Factory
// Binu
// v1.0 05-2017 

#include "DecoderFactory.h"
#include "Decoder_FFMPEG.h"
#include "WMIDecoder.h"

namespace Id_Comp
{

	DecoderFactory::DecoderFactory()
	{
	}


	DecoderFactory::~DecoderFactory()
	{
	}

	// creating decoder based on the codec type
	DecoderBase* DecoderFactory::GetDecoder(CODEC_TYPE codectype_i)
	{
		DecoderBase* pBase = nullptr;
		switch (codectype_i)
		{
			case CODEC_TYPE::FFMPEG:
			{
				pBase = new Decoder_FFMPEG();
				break;
			}
			case CODEC_TYPE::WMI:
			{
				pBase = new WMIDecoder();
				break;
			}
			default:
			{
				pBase = nullptr;
			}
		}
		return pBase;
	}
}
