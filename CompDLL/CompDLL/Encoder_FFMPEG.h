// Encoder FFMPEG
// Binu
// v.1 05-2017 

#pragma once
#include "EncoderBase.h"
#include "H264_ffmpeg.h"
#include "Comp_Logger.h"
#include <thread>

using namespace std;

namespace Id_Comp
{
	class Encoder_FFMPEG : public EncoderBase
	{
	public:
		Encoder_FFMPEG( );
		~Encoder_FFMPEG();

		//inherited functions
		bool Initialize(DataQueue* data_i, int fps,
			int width, int height, int bitrate,
			int video_length_s, int buff_size, Comp_Logger* Logger);
		bool Run(std::string out_path, int encoder);
		int GetCurrentFrame();
		File_Status GetStatus();
		void close();
		void Wait_UntilFinish() {m_thread.join();};
		float GetDuration();
		void ShtdownAll() { m_Shutdown = true; };
	private:
		static void encode_thread(Encoder_FFMPEG* pDecoder); // the encoder thread
		void IncrimentFrame() { m_nCurrentFrame++; };
		//Function to test encoder
		void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);

	private:
		int				m_nEndFrame;
		thread			m_thread;
		H264_ffmpeg*	m_pH264Encoder;
		bool			m_Shutdown;
		bool			m_bAppendPrev = false;
	};
}

