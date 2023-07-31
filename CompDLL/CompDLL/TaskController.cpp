// Task Controller, interace
// Binu
// v.1 05-2017 

#include "TaskController.h"
#include "Commondef.h"
#include "DataQueue.h"
#include "Decoder_FFMPEG.h"
#include "Encoder_FFMPEG.h"
#include "CompressorUtils.h"
#include "AWSUploader.h"
#include "Settings.h"
#include "Comp_Logger.h"
#include "DecoderFactory.h"
#include "EncoderFactory.h"
#include <iostream>
#include <conio.h>
#include <iterator>

using namespace std;

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S"
#define LOGNAME_SIZE 20

namespace Id_Comp
{
	TaskController::TaskController(CODEC_TYPE Codec_type_i):
		m_pAWSUploader(nullptr),
		m_pDecoder(nullptr),
		m_pEncoder(nullptr),
		m_pData(nullptr),
		m_bstarted( false ),
		m_bFinished( false ),
		m_bError( false ),
		m_Shutdown( false ),
		m_InitDone( false )
	{
		m_Logger = new Comp_Logger();

		m_pDecoder = DecoderFactory::GetDecoder( Codec_type_i );
		m_pEncoder = EncoderFactory::GetEncoder(Codec_type_i);
		// logger is not ready at this stage
		//LOG(  m_Logger,  Comp_Logger,LOG_DEBUG, "Entering TaskController::TaskController");
		//LOG(  m_Logger,  Comp_Logger,LOG_DEBUG, "Exiting TaskController::TaskController");
	}


	TaskController::~TaskController()
	{
		//LOG(  m_Logger,LOG_DEBUG, "Entering TaskController::~TaskController");
		try
		{
			if (m_pDecoder)	delete m_pDecoder;
			if (m_pEncoder)		delete m_pEncoder;
			if (m_pData)		delete m_pData;
			if (m_pAWSUploader)
			{
				//m_pAWSUploader->CloseAws();
				delete m_pAWSUploader;
			}
		}
		catch (...)
		{
			LOG(m_Logger, LOG_ERR, "Unexpected Error in TaskController::~TaskController");
		}
		//LOG(  m_Logger,LOG_DEBUG, "Exiting TaskController::~TaskController");

		// always delete logger at end for saftey
		if (m_Logger)		delete m_Logger;
	}

	void TaskController::Close()
	{
		m_Shutdown = true;
		m_InitDone = false;
		this_thread::sleep_for(std::chrono::milliseconds(1000));
		if (m_pDecoder) m_pDecoder->ShtdownAll();
		if (m_pEncoder) m_pEncoder->ShtdownAll();
		if (m_pAWSUploader) m_pAWSUploader->ShtdownAll();
		this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	void TaskController::SetStartTime(int YY, int MM, int DD, int HH, int MN, int SS)
	{
		Settings::GetInstance().SetStartTime(YY, MM, DD, HH, MN, SS);
	}

	void TaskController::SetNetworkDown(bool Down)
	{
		Settings::GetInstance().SetNetworkDown( Down );
	}

	void TaskController::SetEndTime(int YY, int MM, int DD, int HH, int MN, int SS)
	{
		Settings::GetInstance().SetEndTime(YY, MM, DD, HH, MN, SS);
	}

	bool TaskController::Upload_chunk(string in_rawfile, string in_file, string out_aws_path )
	{
		m_bError = false;
		m_Logger->Init(Settings::GetInstance().GetLogLevel(), in_file);
		LOG(m_Logger, LOG_DEBUG, "Entering TaskController::Upload_chunk");

		try
		{
			m_bFinished = false;
			

			
			list<string> item = CompressorUtils::GetAWSPendingItems(in_rawfile);

			if (!m_InitDone)
			{
				m_pAWSUploader = new AWSUploader(item.size(), m_Logger);
				m_pAWSUploader->SetUpConnection();
				m_InitDone = true;
			}
			m_bstarted = true; // shoud be true after aws init
			std::list<string>::iterator it;
			for (it = item.begin(); it != item.end(); ++it) {
				string var = *it;
				vector<string> items = CompressorUtils::split(var, '*');
				m_pAWSUploader->UploadVideo(items[0], items[1], items[2]);
			}

			
			LOG(m_Logger, LOG_INFO, "Please wait ..uploading is going on");
			m_pAWSUploader->WaitToFinish();
			m_bFinished = true;
		}
		catch (...)
		{
			LOG(m_Logger, LOG_ERR, "Unexpected Error in TaskController::Upload_chunk");
		}
		LOG(m_Logger, LOG_DEBUG, "Exiting TaskController::Upload_chunk");

		return true;
	}

	bool TaskController::Process(string in_file, string out_folder_aws,
								 string client_name, string camera_num,
								 bool ignore_ts)
	{
		m_bError = false;
		m_Logger->Init(Settings::GetInstance().GetLogLevel(), in_file);
		LOG(  m_Logger, LOG_DEBUG , "Entering TaskController::Process");

		try
		{

			string out_folder = Settings::GetInstance().GetTempFolder();
			out_folder = CompressorUtils::CreateOutFolder(out_folder, client_name, camera_num);
			m_bFinished = false;
			// This parametrs will read from settings
			int nFrameSkip = Settings::GetInstance().GetFrameSkip();// frames_to_skip;
			int nFrameKeep = Settings::GetInstance().GetFrameKeep();// frames_to_keep
			// Auto calculating bitrate based on resolution
			int nBitrate = Settings::GetInstance().GetBitrate() * ((float)Settings::GetInstance().GetResize() / 100);
			int nSplitLength = Settings::GetInstance().GetSplitLength() * 60; // in seconds
			int nPrevLen = 20; // 2 seconds
			

																			  // Creating Data queue
			m_pData = new DataQueue();
			m_pData->FRAME_BUFFER_SIZE = nPrevLen;

			// Creating and initializing Decoder
			if ( false == m_pDecoder->Initialize(Settings::GetInstance().GetResize(),
				  m_pData, in_file, nFrameSkip, nFrameKeep, m_Logger))
			{
				//failed to read video, already logged the error
				m_bError = true;
				return false;
			}

			int numFramesBuff = m_pDecoder->GetFps() * nPrevLen;

			int nFrames_out = ( m_pDecoder->GetFrameCount() / (float)(nFrameSkip + nFrameKeep)) * nFrameKeep;
			float nChunks = nFrames_out / (m_pDecoder->GetFps() * nSplitLength) + 1;
			m_pAWSUploader = new AWSUploader(nChunks, m_Logger);
			m_pAWSUploader->SetUpConnection();

			// Check for valid resolution
			if (Settings::GetInstance().GetRejectlow())
			{
				if ((m_pDecoder->GetWidth() < 640) || (m_pDecoder->GetHeight() < 480))
				{
					LOG(  m_Logger,LOG_WARN, " The output resolution is lower, rejecting input video");
					LOG(  m_Logger,LOG_DEBUG, "Exiting TaskController::Process");
					m_bError = true;
					return false;
				}
			}

			// User info
			int fps = m_pDecoder->GetFps();

			// Creating encoder
			m_pEncoder->Initialize(m_pData, fps,
				m_pDecoder->GetWidth(), m_pDecoder->GetHeight(),
				nBitrate, nSplitLength, numFramesBuff, m_Logger);

			// Starting decoder thread
			m_pDecoder->Run();

			// number of output videos
			int ncount = 0;
			int encoder = Settings::GetInstance().GetEncoder();
			bool upload = Settings::GetInstance().GetAwsUpload();
			//cout << "Processing " << in_file << endl;
			m_bstarted = true;

			//Getting current time stamp and storing
			Settings::GetInstance().SetCurTime(CompressorUtils::GetCurrDateandTime());
			if (upload)
			{
				m_pAWSUploader->UploadVideo("temp.txt", client_name, camera_num, out_folder_aws, in_file,true);
			}
			// Main loop conttrolling the whole process
			while (m_pDecoder->GetStatus() != FILE_FINISHED) // If file finished
			{
				if (m_Shutdown)
				{
					return true;
				}
				std::ostringstream info;
				info<< "Compressing chunk ID = "<< ncount;
				LOG(  m_Logger,LOG_INFO, info.str());

				if (ncount < 0)
				{
					//append last 2 second video

				}
				// Getting time stamp
				string out = CompressorUtils::getTimeStampedFileNamev2(in_file,
					Settings::GetInstance().GetSplitLength() * 60, ncount, nFrameSkip, nFrameKeep, ignore_ts);

				out = out_folder + "\\" + out;

				// Initializing encoder with new file name
				m_pEncoder->Initialize(m_pData, fps,
					m_pDecoder->GetWidth(), m_pDecoder->GetHeight(),
					nBitrate, nSplitLength, numFramesBuff, m_Logger);
				// Starting encoder thread
				if (false == m_pEncoder->Run(out, encoder))
				{
					//failed to compress, already logged the error
					m_bError = true;
					return false;
				}
				// Waitign to finish encoder thread
				m_pEncoder->Wait_UntilFinish();
				m_pEncoder->close();

				info << "Finished Compression of chunk ID = "<< ncount;

				if (ncount + 1 == nChunks) // last chunk
				{
					// the nuber of frames will be lesser
					//float tot_duration = m_pDecoder->GetFrameCount() / (float)(m_pDecoder->GetFps());
					//float last_duration = tot_duration - (ncount * Settings::GetInstance().GetSplitLength() * 60);
					float last_duration = m_pEncoder->GetDuration();
					string out_new = CompressorUtils::getTimeStampedFileNamev2(in_file,
						Settings::GetInstance().GetSplitLength() * 60, ncount, nFrameSkip, nFrameKeep, ignore_ts, true, (int)last_duration);
					string new_out_path = out_folder + "\\" + out_new;

					// rename file
					rename(out.c_str(), new_out_path.c_str());
					out = new_out_path;
				}

				LOG(  m_Logger,LOG_INFO, info.str());
				// upload to AWS
				if (upload)
				{
					m_pAWSUploader->UploadVideo(out, client_name, camera_num,out_folder_aws, in_file,false);
				}
				//increment video count
				ncount++;
				cout << endl;
			}

			LOG(  m_Logger,LOG_INFO, "Finished the whole Compression");

			if (upload)
			{
				LOG(  m_Logger,LOG_INFO, "Please wait ..uploading is going on");
				m_pAWSUploader->WaitToFinish();
			}
			m_bFinished = true;

			LOG(  m_Logger,LOG_INFO, "Finished the whole uploading");
		}

		catch (...)
		{
			LOG(  m_Logger,LOG_ERR, "Unexpected Error in TaskController::Process");
		}
		LOG(  m_Logger,LOG_DEBUG, "Exiting TaskController::Process");

		return true;
	}

	int TaskController::GetCompressProgress()
	{
		if (m_bError) return -5; // error code
		// dont use log's, because this function will be called many times
		if (m_bstarted)return m_pDecoder->GetProgress();
		else
			return 0;
	}

	int TaskController::GetUploaderProgress()
	{
		// dont use log's, because this function will be called many times
		if (m_bFinished) return -1;
		else
		{
			if (m_bstarted) return m_pAWSUploader->GetProgress();
			else
				return 0;
		}
	}
}
