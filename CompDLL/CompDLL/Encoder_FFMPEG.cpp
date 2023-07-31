// Encoder FFMPEG
// Binu
// v.1 05-2017 

#include "Encoder_FFMPEG.h"
#include <iostream>

using namespace std;

namespace Id_Comp
{
	Encoder_FFMPEG::Encoder_FFMPEG() :
		m_Shutdown( false )
	{
		//LOG(m_Logger, LOG_DEBUG, "Entering TaskController::Encoder_FFMPEG");
		//LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Encoder_FFMPEG");

	}


	Encoder_FFMPEG::~Encoder_FFMPEG()
	{
		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::~Encoder_FFMPEG");
		

		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::~Encoder_FFMPEG");
	}

	bool Encoder_FFMPEG::Initialize(DataQueue* data_i, int fps,
		int width, int height, int bitrate,
		int video_length_s, int buff_size, Comp_Logger* Logger)
	{
		m_pData			= data_i; //shallow copying
		m_nFps			= fps;
		m_nWidth		= width;
		m_nHeight		= height;
		m_nBitrate		= bitrate;
		m_video_length_s  = video_length_s;
		m_nCurrentFrame  = 0;
		m_Logger		= Logger;
		m_buffer_size	= buff_size;

		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::Initialize()");
		m_pH264Encoder = new H264_ffmpeg(m_Logger);
		m_nEndFrame = m_nFps * m_video_length_s;
		av_log_set_callback(avlog_cb);
		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Initialize()");
		return true;
	}

	void Encoder_FFMPEG::close()
	{
		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::close()");
		//Close Video
		m_pH264Encoder->CloseVideo();

		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::close()");
		delete m_pH264Encoder;
	}
	bool Encoder_FFMPEG::Run(std::string out_path, int encoder)
	{
		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::Run()");
		m_Filepath = out_path;
		m_nCurrentFrame = 0;

		AVCodecID codec = AV_CODEC_ID_MPEG2VIDEO;
		if (encoder == 2)
		{
			codec = AV_CODEC_ID_H264;
		}
		//Initialise Video
		if (false == m_pH264Encoder->SetupVideo((char*)m_Filepath.c_str(), m_nWidth,
			m_nHeight, m_nFps, 1, m_nBitrate, codec))
		{
			LOG(m_Logger, LOG_ERR, "Error - failed to create encoder");
			LOG(m_Logger, LOG_DEBUG, "Exiting Encoder_FFMPEG::Run");
			return false;
		}
			
		//creating thread
		m_thread = thread(&encode_thread, this);

		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Run()");
		return true;
	}

	int Encoder_FFMPEG::GetCurrentFrame()
	{
		return m_nCurrentFrame;
	}

	File_Status Encoder_FFMPEG::GetStatus()
	{
		return m_pData->GetFileStatus();
	}

	// only for debug purpose
	void Encoder_FFMPEG::SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
	{
		FILE *pFile;
		char szFilename[32];
		int  y;

		// Open file
		sprintf(szFilename, "frame%d.ppm", iFrame);
		pFile = fopen(szFilename, "wb");
		if (pFile == NULL)
			return;

		// Write header
		fprintf(pFile, "P6\n%d %d\n255\n", width, height);

		// Write pixel data
		for (y = 0; y<height; y++)
			fwrite(pFrame->data[0] + y*pFrame->linesize[0], 1, width * 3, pFile);

		// Close file
		fclose(pFile);
	}

	// Main thread for performing encoding
	void Encoder_FFMPEG::encode_thread(Encoder_FFMPEG* pEncoder)
	{
		LOG(pEncoder->m_Logger, LOG_DEBUG, "Entering TaskController::encode_thread()");
		FrameData* pFrameData = NULL;  // frame data
		try
		{
			AVFrame* pFrameRGB = NULL; // RGB Data

			bool initbuff = false;
			//Encoder thread
			while (pEncoder->m_nCurrentFrame != pEncoder->m_nEndFrame)
			{
				if (pEncoder->m_Shutdown)
				{
					return;
				}
				//Getting last 2sec frames
				if ((pEncoder->m_bAppendPrev == true) )
				{
					pFrameData = pEncoder->m_pData->GetPrevBuffer();
					//static int i = 0;
					//pEncoder->SaveFrame(pFrameData->GetData(), pEncoder->m_nWidth, pEncoder->m_nHeight, i++);
					if (pFrameData == NULL)
					{
						pEncoder->m_bAppendPrev = false;
						continue;
					}
					
				}
				else
				{
					pEncoder->m_bAppendPrev = false;
					// Getting RGB frame from queue
					pFrameData = pEncoder->m_pData->GetReadBuffer();
					if (pFrameData == NULL)
					{
						if (pEncoder->m_pData->GetFileStatus() == FILE_FINISHED)
						{
							break;
						}
						// to reduce CPU utilization from idle loops
						this_thread::sleep_for(std::chrono::milliseconds(1));
						continue;

					}		

					pEncoder->IncrimentFrame();
				}

				pFrameRGB = pFrameData->GetData();

				if (pFrameRGB == nullptr)
				{
					return;
				}

				pEncoder->m_pH264Encoder->WriteFrame(pFrameRGB);

				/*if ((pEncoder->m_bAppendPrev == false) && (pEncoder->m_nCurrentFrame > (pEncoder->m_nEndFrame- pEncoder->m_buffer_size)))
				{
					pFrameData->SetStatus(FRAME_BUFF_READY);
					if (initbuff == false)
					{
						pEncoder->m_pData->SetPrevPos(pEncoder->m_nCurrentFrame);
						initbuff = true;
					}
				}
				else*/
				{
					// Set the ready status to the frame
					pFrameData->SetStatus(FRAME_WRITE_READY);
				}

				pFrameData = NULL;
				pFrameRGB = NULL;
				//pEncoder->IncrimentFrame();
				//cout << "\r Compressing " << 
				//	int(100 * (pEncoder->m_nCurrentFrame/ (double)pEncoder->m_nEndFrame)) << "%";
			}  //end of while

			//pEncoder->m_bAppendPrev = true;
		}
		catch (...)
		{
			pFrameData->SetStatus(FRAME_WRITE_READY);
			LOG(pEncoder->m_Logger, LOG_WARN, "Unexpected error occured in TaskController::encode_thread()");
		}
		LOG(pEncoder->m_Logger, LOG_DEBUG, "Exiting TaskController::encode_thread()");
		return;
	}

	float Encoder_FFMPEG::GetDuration()
	{
		return (m_nCurrentFrame / (float)m_nFps);
	}

}
