// Encoder base
// Binu
// v.1 05-2017 

#include "EncoderBase.h"

namespace Id_Comp
{

	EncoderBase::EncoderBase():
		m_pData(nullptr), //shallow copying
		m_nFps(30),
		m_nWidth(0),
		m_nHeight(0),
		m_nBitrate(50000),
		m_video_length_s(600),
		m_nCurrentFrame(0),
		m_Logger(nullptr),
		m_buffer_size(16)

	{
	}
}
