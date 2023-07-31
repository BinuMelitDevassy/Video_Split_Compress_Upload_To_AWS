// Encoder Factory
// Binu
// v1.0 05-2017 

#include "EncoderFactory.h"
#include "Encoder_FFMPEG.h"
#include "WMIEncoder.h"

namespace Id_Comp
{
	EncoderFactory::EncoderFactory()
	{
	}


	EncoderFactory::~EncoderFactory()
	{
	}

	EncoderBase* EncoderFactory::GetEncoder(CODEC_TYPE codectype_i)
	{
		EncoderBase* pBase = nullptr;
		switch (codectype_i)
		{
			case CODEC_TYPE::FFMPEG:
			{
				pBase = new Encoder_FFMPEG();
				break;
			}
			case CODEC_TYPE::WMI:
			{
				pBase = new WMIEncoder();
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
