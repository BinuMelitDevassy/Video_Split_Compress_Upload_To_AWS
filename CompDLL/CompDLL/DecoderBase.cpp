// Encoder base
// Binu
// v.1 05-2017 

#include "DecoderBase.h"

namespace Id_Comp
{

	DecoderBase::DecoderBase():
	m_Filepath(""),
	m_pData(nullptr), //shallow copying
	m_nFps(30), //default
	m_nWidth(0),
	m_nHeight(0),
	m_nCurrentFrame(0),
	m_nFramesToSkip(0),
	m_nFramesToKeep(0),
	m_nTotalFrames(0),
	m_Logger( nullptr )
	{
	}

}
