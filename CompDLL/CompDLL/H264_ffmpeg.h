
// H264 Encoder
// Binu
// v.1 05-2017 

#pragma once
#include "Comp_Logger.h"

#define _CRT_SECURE_NO_WARNINGS

// FFMPEG is writen in C so we need to use extern "C"
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
	
	static int sws_flags = SWS_BICUBIC;
}

using namespace Id_Comp;
// FFMPEG h264/mpeg2 encoder
// Class core logic is taken from ffmpeg sample code
class H264_ffmpeg
{
public:
	//-----------------------------
	//Define our subroutines that we want the outside world to access
	//-----------------------------

	H264_ffmpeg(Comp_Logger* Logger);
	~H264_ffmpeg();

	bool SetupVideo(char * filename, int Width, int Height, int FPS,
					 int GOB, int BitPerSecond, AVCodecID codec_id);
	void WriteFrame(AVFrame * RGBFrame);
	void CloseVideo(void);
	
	int GetVideoWidth(void) {return m_AVIMOV_WIDTH;}
	int GetVideoHeight(void) {return m_AVIMOV_HEIGHT;}
	
private:

	int m_sws_flags;
	int	m_AVIMOV_FPS;
	int	m_AVIMOV_GOB;
	int	m_AVIMOV_BPS;
	int m_frame_count;
	int	m_AVIMOV_WIDTH;
	int	m_AVIMOV_HEIGHT;

	bool m_AVIMutex;

    double m_video_time;

	char *m_filename;

	void CloseCodec(void);
	bool SetupCodec(const char *filename, AVCodecID codec_id);

	AVFrame *m_frame;
    AVCodecContext *m_c;
	AVStream *m_video_st;
	AVOutputFormat *m_fmt;
	AVFormatContext *m_oc;
	AVCodec *m_video_codec;
	Comp_Logger* m_Logger;
};
