
// FFMPEG Encoder
// Binu
// v.1 05-2017 

#include "Decoder_FFMPEG.h"
#include "Settings.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <math.h>


using namespace std;

namespace Id_Comp
{

	Decoder_FFMPEG::Decoder_FFMPEG():
	m_Shutdown( false )
	{
		//LOG(m_Logger, LOG_DEBUG, "Entering TaskController::Decoder_FFMPEG");
		//LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Decoder_FFMPEG");
	}


	Decoder_FFMPEG::~Decoder_FFMPEG()
	{
		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::~Decoder_FFMPEG");
		if (m_thread.joinable())
		{
			m_thread.join();
		}
		// Free the RGB image
		av_free(m_buffer);
		
		// Close the codecs
		avcodec_close(m_pCodecCtx);

		// Close the video file
		avformat_close_input(&m_pFormatCtx);
		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::~Decoder_FFMPEG");
	}

	double Decoder_FFMPEG::r2d(AVRational r)
	{
		return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
	}


	bool Decoder_FFMPEG::Initialize( int resize, DataQueue* data_i, std::string in_path, int framestoskip,
		int framestokeep, Comp_Logger* Logger)
	{
		m_Filepath		= in_path;
		m_pData			= data_i; //shallow copying
		m_nFramesToSkip	= framestoskip;
		m_nFramesToKeep = framestokeep;
		m_Logger		= Logger;

		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::Initialize");
		// Register all formats and codecs
		av_register_all();
		av_log_set_callback(avlog_cb);

		// Open video file
		if (avformat_open_input(&m_pFormatCtx, m_Filepath.c_str(), NULL, NULL) != 0)
		{
			LOG(m_Logger, LOG_ERR, "Failed to Open input video");
			LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
			return false; // Couldn't open file
		}

		// Retrieve stream information
		if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0)
		{
			LOG(m_Logger, LOG_ERR, "Failed to Open stream info");
			LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
			return false; // Couldn't find stream information
		}
		
		// Dump information about file onto standard error
		av_dump_format( m_pFormatCtx, 0, m_Filepath.c_str(), 0);

		// Find the first video stream
		m_nVideoStream = -1;
		for ( int i = 0; i< m_pFormatCtx->nb_streams; i++)
			if ( m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
				m_nVideoStream = i;
				m_gStream = m_pFormatCtx->streams[i];
				break;
			}
		if (m_nVideoStream == -1)
		{
			LOG(m_Logger, LOG_ERR, "Failed to find a video stream");
			LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
			return false; // Didn't find a video stream
		}

		// Get a pointer to the codec context for the video stream
		AVCodecContext  *pCodecCtxOrig = NULL;
		pCodecCtxOrig = m_pFormatCtx->streams[m_nVideoStream]->codec;

		m_nFps = ceil(r2d(m_pFormatCtx->streams[m_nVideoStream]->r_frame_rate));

		if (m_nFps < Settings::GetInstance().GetMinFPS())
		{
			if (Settings::GetInstance().GetRejectFps())
			{
				LOG(m_Logger, LOG_ERR, "Unsupported fps");
				LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
				return false; // Codec not found
			}
			else
			{
				m_nFps = Settings::GetInstance().GetMinFPS();
			}
		}

		if (m_nFps > Settings::GetInstance().GetMaxFPS())
		{
			if (Settings::GetInstance().GetRejectFps())
			{
				LOG(m_Logger, LOG_ERR, "Unsupported fps");
				LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
				return false; // Codec not found
			}
			else
			{
				m_nFps = Settings::GetInstance().GetMaxFPS();
			}
		}

		m_nTotalFrames = m_pFormatCtx->streams[m_nVideoStream]->nb_frames;
		if (m_nTotalFrames < 1) // invalid
		{
			m_nTotalFrames = m_pFormatCtx->streams[m_nVideoStream]->duration;
		}
		// Find the decoder for the video stream
		AVCodec *pCodec = NULL;
		pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
		if (pCodec == NULL) {
			LOG(m_Logger, LOG_ERR, "Unsupported codec");
			LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
			return false; // Codec not found
		}

		if (m_pCodecCtx == NULL) {
			// Codec not found, switch to vailable codec
		}

		// Copy context
		m_pCodecCtx = avcodec_alloc_context3(pCodec);
		if (avcodec_copy_context(m_pCodecCtx, pCodecCtxOrig) != 0) {
			LOG(m_Logger, LOG_ERR, "Couldn't copy codec context");
			LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
			return false; // Error copying codec context
		}

		// Open codec
		if (avcodec_open2(m_pCodecCtx, pCodec, NULL) < 0)
		{
			LOG(m_Logger, LOG_ERR, "Could not open codec");
			LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");
			return false; // Could not open codec
		}

		int  numBytes;
	
		// Determine required buffer size and allocate buffer
		numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, m_pCodecCtx->width,
									  m_pCodecCtx->height);
		m_buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

		//Reading values
		if (resize > 0)
		{
			m_nWidth = m_pCodecCtx->width * ((float)resize / 100.0);
			m_nHeight = m_pCodecCtx->height * ((float)resize / 100.0);
		}

		// Initializing RGB Frames
		m_pData->InitializeDataFrames(m_buffer, m_nWidth, m_nHeight, m_nFps);
		
		// Releasing dummy context
		avcodec_close(pCodecCtxOrig);

		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize");;
		return true;
	}

	void Decoder_FFMPEG::Run()
	{
		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::Run");;
		//creating thread
		m_thread = thread(&decode_thread, this);
		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Run");;
  	}
	
	int Decoder_FFMPEG::GetCurrentFrame()
	{
		return m_nCurrentFrame;
	}

	File_Status Decoder_FFMPEG::GetStatus()
	{
		return m_pData->GetFileStatus();
	}
 
	void Decoder_FFMPEG::decode_thread(Decoder_FFMPEG* pDecoder)
	{
		LOG(pDecoder->m_Logger, LOG_DEBUG, "Entering TaskController::decode_thread");;
		FrameData* pFrameData = NULL;  // frame data
		try
		{
			// Setting file status
			pDecoder->m_pData->SetFileStatus(FILE_PROCESSING);
			//Creating elements for decoding
			// Creating frame buffer
			AVFrame*  pFrame = NULL;
			pFrame = av_frame_alloc();
			AVPacket  packet;
			int       frameFinished;
			struct SwsContext *sws_ctx = NULL;
			AVFrame* pFrameRGB = NULL; // RGB Data
			

			// initialize SWS context for software scaling
			sws_ctx = sws_getContext(pDecoder->m_pCodecCtx->width,
				pDecoder->m_pCodecCtx->height,
				pDecoder->m_pCodecCtx->pix_fmt,
				pDecoder->m_nWidth,
				pDecoder->m_nHeight,
				AV_PIX_FMT_YUV420P,
				SWS_BILINEAR,
				NULL,
				NULL,
				NULL
			);

			//Main thread loop - reading frame
			bool reset_time = true;
			static chrono::steady_clock::time_point start;
			int original_frame_no = 0;
			bool bskip = false;
			int nskipped = 0;
			int nMax = 9000-1;
			int nCurrent = 0;
			int nkeep = 0; // keeped frames
			while (1) {
				if (pDecoder->m_Shutdown)
				{
					return;
				}
				if (pFrameData != NULL)
				{
					pFrameData->SetStatus(FRAME_WRITE_READY);
				}

				if (nCurrent >= nMax)
				{
					nCurrent = 0;
					pDecoder->SeekBack(150);
				}
				// Getting RGB frame from queue
				pFrameData = pDecoder->m_pData->GetWriteBuffer();

				if (pFrameData == NULL)
				{
					int retry = 0;
					while (retry < 1000)
					{
						pFrameData = pDecoder->m_pData->GetWriteBuffer();
						if (pFrameData != NULL)
						{
							break;
						}
						retry++;
						// to reduce CPU utilization from idle loops
						this_thread::sleep_for(std::chrono::milliseconds(1));
					}

					//continue;
				}

				if (av_read_frame(pDecoder->m_pFormatCtx, &packet) < 0)
				{
					break;
				}

				//Getting the time elapsed
				if (reset_time)
				{
					start = std::chrono::high_resolution_clock::now();
					reset_time = false;
				}

				// Is this a packet from the video stream?
				if (packet.stream_index == pDecoder->m_nVideoStream) {
					// Decode video frame
					avcodec_decode_video2(pDecoder->m_pCodecCtx, pFrame,
						&frameFinished, &packet);

					// Did we get a video frame?
					if (frameFinished) 
					{
						pDecoder->IncrimentFrame();
						
						//checking skip
						if (bskip && ( nskipped < pDecoder->m_nFramesToSkip ))
						{
							nskipped++;
							av_free_packet(&packet);
							pFrameData->SetStatus(FRAME_WRITE_READY);
							continue;
						}
						else
						{
							bskip = false;
							nskipped = 0;
						}


						pFrameRGB = pFrameData->GetData();
						pFrameRGB->pts = pDecoder->m_nCurrentFrame;
						// Convert the image from its native format to YUV
						sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
							pFrame->linesize, 0, pDecoder->m_pCodecCtx->height,
							pFrameRGB->data, pFrameRGB->linesize);
						nCurrent++;
						// Set the ready status to the frame
						pFrameData->SetStatus(FRAME_READ_READY);
						pFrameData = NULL;
						pFrameRGB = NULL;
						nkeep++; //incrementing keeped frames
						//std::cout << "Decoding --" << pDecoder->m_nCurrentFrame << endl;
						
						if(( pDecoder->m_nFramesToSkip > 0 ) && (nkeep >= pDecoder->m_nFramesToKeep))
						{
							/*int flags = AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD;
							original_frame_no += pDecoder->m_nFramesToSkip + 1;
							av_seek_frame(pDecoder->m_pFormatCtx, pDecoder->m_nVideoStream, original_frame_no, flags);*/
							nkeep = 0;
							nskipped = 0;
							bskip = true;
						}

						auto end = std::chrono::high_resolution_clock::now();
						auto elapsed = (end - start).count() / (1000000); //in ms
						auto single_duration = 1000 / pDecoder->m_nFps;
						auto wait_time = single_duration - elapsed;
						//this_thread::sleep_for(std::chrono::milliseconds(wait_time));
						//cout << "\r processing frame = "<< pDecoder->m_nCurrentFrame << endl;
						//this_thread::sleep_for(std::chrono::milliseconds(1));
						reset_time = true;
					}
					else
					{
						pFrameData->SetStatus(FRAME_WRITE_READY);
						pFrameData = NULL;
						pFrameRGB = NULL;
					}
				}
				else
				{
					pFrameData->SetStatus(FRAME_WRITE_READY);
					pFrameData = NULL;
					pFrameRGB = NULL;
				}
				// Free the packet that was allocated by av_read_frame
				av_free_packet(&packet);
			} // while end
			av_frame_free(&pFrame);

			pDecoder->m_pData->SetFileStatus(FILE_FINISHED);
		}
		catch (...)
		{
			LOG(pDecoder->m_Logger, LOG_ERR, "Unexpected Error TaskController::decode_thread");;
			pFrameData->SetStatus(FRAME_WRITE_READY);
		}
		LOG(pDecoder->m_Logger, LOG_DEBUG, "Exiting TaskController::decode_thread");;
	}

	int Decoder_FFMPEG::GetProgress()
	{
		if (m_nTotalFrames)return (m_nCurrentFrame /(float)m_nTotalFrames) * 100;
		else
			return 0;
	}

	/*double get_master_clock()
	{
		
		if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
			return get_video_clock(is);
		}
		else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
			return get_audio_clock(is);
		}
		else {
			return get_external_clock(is);
		}
	}*/

	void Decoder_FFMPEG::SeekBack(int64_t  seek)
	{
		avformat_seek_file(m_pFormatCtx, -1, seek, seek, INT64_MAX, AVSEEK_FLAG_BACKWARD);
		m_nCurrentFrame = m_nCurrentFrame - (int)seek*2;
		/*int stream_index = -1;
		int video_current_pts_time = av_gettime();
		int pos = get_master_clock(global_video_state);
		pos += incr;
		stream_seek(global_video_state, (int64_t)(pos * AV_TIME_BASE), incr);
		int64_t seek_target = is->seek_pos;

		if (is->videoStream >= 0) stream_index = is->videoStream;

		if (stream_index >= 0) {
			seek_target = av_rescale_q(seek_target, AV_TIME_BASE_Q,
				pFormatCtx->streams[stream_index]->time_base);
		}
		if (av_seek_frame(is->pFormatCtx, stream_index,
			seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
			fprintf(stderr, "%s: error while seeking\n",
				is->pFormatCtx->filename);
		}
		else {

			if (is->videoStream >= 0) {
				packet_queue_flush(&is->videoq);
				packet_queue_put(&is->videoq, &flush_pkt);
			}
		}*/

	}

}
