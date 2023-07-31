// FFMPEG Encoder
// Binu
// v.1 05-2017 

#pragma once
#include "DecoderBase.h"
#include <thread>


using namespace std;


extern "C" {
#define INT64_C(x) (x ## LL)
#define UINT64_C(x) (x ## ULL)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
}

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif


namespace Id_Comp
{
	//FFMPEG Decoder class
	class Decoder_FFMPEG : public DecoderBase
	{
	public:
		Decoder_FFMPEG();
		~Decoder_FFMPEG();

		//virtual functions from base
		virtual bool Initialize( int resize, DataQueue* data_i, std::string in_path, int framestoskip,
			int framestokeep, Comp_Logger* Logger);
		virtual void Run();
		virtual int GetCurrentFrame();
		virtual File_Status GetStatus();
		virtual int GetProgress();
		AVStream* GetGstream() { return m_gStream; };
		void ShtdownAll() { m_Shutdown = true; };

	private:
		static void decode_thread( Decoder_FFMPEG* pDecoder ); // the decoder thread
		double r2d(AVRational r); // AV r conversion function
		void IncrimentFrame() { m_nCurrentFrame++; };
		void SeekBack(int64_t  seek);
	private:

		AVFormatContext*  m_pFormatCtx = NULL;
		int               m_nVideoStream;
		AVCodecContext*   m_pCodecCtx = NULL;
		uint8_t*		  m_buffer = NULL;
		thread			  m_thread;
		AVStream*		  m_gStream;
		bool			  m_Shutdown;
	};
}

