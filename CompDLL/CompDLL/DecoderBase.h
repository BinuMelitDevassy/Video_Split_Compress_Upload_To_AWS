
// Decoder base
// Binu
// v.1 05-2017 

#pragma once

#include "DataQueue.h"
#include "Commondef.h"
#include "Comp_Logger.h"

namespace Id_Comp
{
	// All decoders should inherit from this class. Currently we are
	// implementing only ffmpeg, in future need implementation using MWI
	// All need to follow the interface specifications
	class DecoderBase
	{
	public:
		DecoderBase();
		virtual ~DecoderBase() = default;

		virtual bool Initialize(int resize, DataQueue* data_i, std::string in_path, int framestoskip,
			int framestokeep, Comp_Logger* Logger) = 0;
		virtual void Run() = 0;
		virtual int GetCurrentFrame() = 0;
		virtual File_Status GetStatus() = 0;
		virtual int GetProgress() = 0;

		int GetWidth() { return m_nWidth; };
		int GetHeight() { return m_nHeight; };
		int GetFps() { return m_nFps; };
		int GetFrameCount() { return m_nTotalFrames; };
		virtual void ShtdownAll() = 0;

	protected:

		std::string m_Filepath;
		int			m_nFps;
		int			m_nCurrentFrame;
		int			m_nWidth;
		int			m_nHeight;
		int			m_nFramesToSkip;
		int			m_nFramesToKeep;
		int			m_nTotalFrames;
		DataQueue*	m_pData;
		Comp_Logger* m_Logger;
	};
}

