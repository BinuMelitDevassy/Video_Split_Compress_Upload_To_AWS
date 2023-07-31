// Encoder base
// Binu
// v.1 05-2017 

#pragma once

#include "DataQueue.h"
#include "Commondef.h"
#include "Comp_Logger.h"

#include <string>

namespace Id_Comp
{
	// All encoders should inherit from this class. Currently we are
	// implementing only ffmpeg.in future need implementation using MWI
	// All need to follow the interface specifications
	class EncoderBase
	{
	public:
		EncoderBase();
		virtual ~EncoderBase() = default;
		virtual bool Initialize( DataQueue* data_i, int fps,
			int width, int height, int bitrate, int video_length_s,
			int buffer_size, Comp_Logger* Logger ) = 0;
		virtual bool Run(std::string out_path, int encoder) = 0;
		virtual int GetCurrentFrame() = 0;
		virtual File_Status GetStatus() = 0;
		virtual void Wait_UntilFinish() = 0;
		virtual void close() = 0;
		virtual float GetDuration() = 0;
		virtual void ShtdownAll() = 0;

	protected:

		std::string m_Filepath;
		int			m_nFps;
		int			m_nCurrentFrame;
		int			m_nWidth;
		int			m_nHeight;
		int			m_nBitrate;
		int			m_video_length_s; // video length in seconds
		DataQueue*	m_pData;
		Comp_Logger* m_Logger;
		int m_buffer_size;
	};
}

