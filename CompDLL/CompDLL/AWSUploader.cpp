// AWS Uploader class header
// Binu
// v1.0 05-2017 

#include "AWSUploader.h"
#include "Settings.h"
#include "Comp_Logger.h"
#include "CompressorUtils.h"

#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/s3/model/AbortMultipartUploadRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/ListMultipartUploadsRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/utils/ratelimiter/DefaultRateLimiter.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/PutBucketVersioningRequest.h>
#include <aws/s3/model/ListObjectVersionsRequest.h>
#include <aws/core/platform/FileSystem.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/platform/Platform.h>
#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/Region.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <fstream>
#include <time.h>

#include <aws/core/utils/logging/LogMacros.h>

using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace Aws::Transfer;
using namespace Aws::Client;
using namespace Aws::Http;
using namespace Aws::Utils;
using namespace Aws::Auth;
using namespace std;

namespace Id_Comp
{
	Aws::SDKOptions						g_options;

	AWSUploader::AWSUploader(float nChunks, Comp_Logger* Logger) :
		m_fChunks( nChunks ),
		m_Shutdown(false),
		m_sAccesKeyID("needan id"),
		m_sSecretKey("needa key"),
		m_AwsThreads(nullptr),
		m_nFinished(0),
		m_Logger( Logger )
	{
		LOG(m_Logger,LOG_DEBUG, "Entering AWSUploader::AWSUploader");
		m_nThreadCount = Settings::GetInstance().GetAwsThreads();
		LOG(m_Logger,LOG_DEBUG, "Exiting AWSUploader::AWSUploader");
	}


	AWSUploader::~AWSUploader()
	{
		LOG(m_Logger,LOG_DEBUG, "Entering AWSUploader::AWSUploader");

		try
		{
			for (int n = 0; n < m_nThreadCount; n++)
			{
				delete  m_AwsThreads[n].TrnsMgr;
				if (m_AwsThreads[n].thread.joinable())
				{
					m_AwsThreads[n].thread.join();
				}
			}
			delete[] m_AwsThreads;
		}
		catch (...)
		{
			LOG(m_Logger,LOG_ERR, "Unexpected Error in UnAWSUploader::~AWSUploader");
		}

		LOG(m_Logger,LOG_DEBUG, "Exiting AWSUploader::~AWSUploader");
	}

	// This function will setup AWS coonection
	bool AWSUploader::SetUpConnection()
	{
		LOG(m_Logger, LOG_DEBUG, "Entering AWSUploader::SetUpConnection");

		g_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
		Aws::InitAPI(g_options);

		// Create a client
		ClientConfiguration config;
		config.scheme = Scheme::HTTP;
		config.connectTimeoutMs = 3000;
		config.requestTimeoutMs = 60000;
		config.region = "XXX";
		m_sBucketName = "XXX";

		AWSCredentials credetials;
		credetials.SetAWSAccessKeyId(m_sAccesKeyID.c_str());
		credetials.SetAWSSecretKey(m_sSecretKey.c_str());

		TransferManagerConfiguration	TrnsMgrConfig;
		
		m_AwsThreads = new Uploader_Params[m_nThreadCount];

		for (int n = 0; n < m_nThreadCount; n++)
		{
			m_AwsThreads[n].s3Client = Aws::MakeShared<S3Client>("AWSUploader", credetials, config, false);
			if (m_AwsThreads[n].s3Client == nullptr)
			{
				return false;
			}
			TrnsMgrConfig.s3Client = m_AwsThreads[n].s3Client;
			m_AwsThreads[n].TrnsMgr = new TransferManager(TrnsMgrConfig);
		}

		LOG(m_Logger, LOG_DEBUG, "Exiting AWSUploader::SetUpConnection");

		return true;
	}

	void AWSUploader::WaitToFinish()
	{
		LOG(m_Logger, LOG_DEBUG, "Entering AWSUploader::WaitToFinish()");
		for (int aws_count = 0; aws_count < m_nThreadCount; aws_count++)
		{
			if (m_AwsThreads[aws_count].thread.joinable())
			{
				m_AwsThreads[aws_count].thread.join();
			}
		}
		LOG(m_Logger, LOG_DEBUG, "Exiting AWSUploader::WaitToFinish");
	}

	int AWSUploader::GetProgress()
	{
		//LOG(m_Logger, LOG_DEBUG, "Entering AWSUploader::GetProgress");
		int total_cur_prog = 0;
		for (int aws_count = 0; aws_count < m_nThreadCount; aws_count++)
		{
			if ((m_AwsThreads[aws_count].status == AWS_Uploading) && 
				(m_AwsThreads[aws_count].btext == false))
			{
				total_cur_prog += m_AwsThreads[aws_count].progress / m_fChunks;
			}
		}

		//total_cur_prog = total_cur_prog / 2;

		int prog = (m_nFinished / m_fChunks) * 100;
		prog += total_cur_prog;

		if ( prog > 99 )
		{
			prog = 99;
		}

		//LOG(m_Logger, LOG_DEBUG, "Exiting AWSUploader::GetProgress");

		return prog;
	}

	//upload pending items
	bool AWSUploader::UploadVideo(string raw_filename, string in_file, string out_aws_path)
	{
		LOG(m_Logger, LOG_DEBUG, "Entering AWSUploader::UploadVideo_pending");
		for (int aws_count = 0; aws_count < m_nThreadCount; aws_count++)
		{
			if (m_AwsThreads[aws_count].status == AWS_Free)
			{
				m_AwsThreads[aws_count].aws_path_full = out_aws_path;
				m_AwsThreads[aws_count].in_path_full = in_file;
				m_AwsThreads[aws_count].status = AWS_Uploading;
				m_AwsThreads[aws_count].BucketName = m_sBucketName;
				m_AwsThreads[aws_count].upload_pending = true;
				m_AwsThreads[aws_count].uploader = this;
				m_AwsThreads[aws_count].btext = false;
				m_AwsThreads[aws_count].raw_with_extn = raw_filename;
				//creating thread
				if (m_AwsThreads[aws_count].thread.joinable())
				{
					m_AwsThreads[aws_count].thread.join();
				}
				m_AwsThreads[aws_count].thread = thread(&aws_thread, &m_AwsThreads[aws_count]);
				LOG(m_Logger, LOG_DEBUG, "Exiting - True- AWSUploader::UploadVideo");
				return true;
			}
		}

		//all threads are busy, retry 50 times
		for (int ncount = 0; ncount < 50; ncount++)
		{
			this_thread::sleep_for(std::chrono::milliseconds(30000));
			if (UploadVideo(raw_filename, in_file, out_aws_path))
			{
				LOG(m_Logger, LOG_DEBUG, "EXiting - True- AWSUploader::UploadVideo");
				return true;
			}
		}
		return false;
	}

	bool AWSUploader::UploadVideo(string file_path, string client, string cam,
								  string out_folder, string raw_file_name, bool bText )
	{
		LOG(m_Logger, LOG_DEBUG, "Entering AWSUploader::UploadVideo");

		string filename = raw_file_name;
		const size_t last_slash_idx = filename.find_last_of("\\/");
		if (std::string::npos != last_slash_idx)
		{
			filename.erase(0, last_slash_idx + 1);
		}

		for (int aws_count = 0; aws_count < m_nThreadCount; aws_count++)
		{
			if (m_AwsThreads[aws_count].status == AWS_Free)
			{
				m_AwsThreads[aws_count].file_path = file_path;
				m_AwsThreads[aws_count].client = client;
				m_AwsThreads[aws_count].cam = cam;
				m_AwsThreads[aws_count].BucketName = m_sBucketName;
				m_AwsThreads[aws_count].status = AWS_Uploading;
				m_AwsThreads[aws_count].uploader = this;
				m_AwsThreads[aws_count].btext = bText;
				m_AwsThreads[aws_count].out_folder = out_folder;
				m_AwsThreads[aws_count].raw_with_extn = filename;
				m_AwsThreads[aws_count].raw_file_name = filename.substr(0, filename.size() - 4); // remove extnraw_file_name;
				//creating thread
				if (m_AwsThreads[aws_count].thread.joinable())
				{
					m_AwsThreads[aws_count].thread.join();
				}
				m_AwsThreads[aws_count].thread = thread(&aws_thread, &m_AwsThreads[aws_count]);
				LOG(m_Logger, LOG_DEBUG, "Exiting - True- AWSUploader::UploadVideo");
				return true;
			}
			else if (m_AwsThreads[aws_count].status == AWS_Error)
			{
				//log error
				m_AwsThreads[aws_count].file_path = file_path;
				m_AwsThreads[aws_count].client = client;
				m_AwsThreads[aws_count].cam = cam;
				m_AwsThreads[aws_count].BucketName = m_sBucketName;
				m_AwsThreads[aws_count].status = AWS_Uploading;
				m_AwsThreads[aws_count].uploader = this;
				m_AwsThreads[aws_count].btext = bText;
				m_AwsThreads[aws_count].out_folder = out_folder;
				m_AwsThreads[aws_count].raw_with_extn = filename;
				m_AwsThreads[aws_count].raw_file_name = filename.substr(0, filename.size() - 4); // remove extnraw_file_name;
				if (m_AwsThreads[aws_count].thread.joinable())
				{
					m_AwsThreads[aws_count].thread.join();
				}
				m_AwsThreads[aws_count].thread = thread(&aws_thread, &m_AwsThreads[aws_count]);
				LOG(m_Logger, LOG_DEBUG, "EXiting - True- AWSUploader::UploadVideo");
				return true;
			}
		}

		//all threads are busy, retry 50 times
		for (int ncount = 0; ncount < 50; ncount++)
		{
			this_thread::sleep_for(std::chrono::milliseconds(5000));
			if (UploadVideo(file_path, client, cam, out_folder, raw_file_name, false))
			{
				LOG(m_Logger, LOG_DEBUG, "EXiting - True- AWSUploader::UploadVideo");
				return true;;
			}
		}

		LOG(m_Logger, LOG_DEBUG, "EXiting - False- AWSUploader::UploadVideo");

		return false;
	}

	void AWSUploader::aws_thread(Uploader_Params* pUploader)
	{
		LOG(pUploader->uploader->m_Logger, LOG_DEBUG, "Entering AWSUploader::aws_thread");

		try
		{
			shared_ptr<TransferHandle> requestPtr = 0;
			string folder_path = "";
			if (pUploader->upload_pending) // upload pending
			{
				requestPtr = pUploader->TrnsMgr->UploadFile(pUploader->in_path_full.c_str(), pUploader->BucketName.c_str(),
					pUploader->aws_path_full.c_str(), "text/plain", Aws::Map<Aws::String, Aws::String>());
			}
			else
			{
				// Extracting filename
				string filename = pUploader->file_path;
				const size_t last_slash_idx = filename.find_last_of("\\/");
				if (std::string::npos != last_slash_idx)
				{
					filename.erase(0, last_slash_idx + 1);
				}


				if (pUploader->btext) // if txt file
				{
					filename = pUploader->out_folder + ".txt";
				}

				folder_path = pUploader->client + "/" + pUploader->cam + "/" + pUploader->out_folder
					+ "/" + pUploader->raw_file_name + "/" + filename;
				requestPtr = pUploader->TrnsMgr->UploadFile(pUploader->file_path.c_str(), pUploader->BucketName.c_str(),
					folder_path.c_str(), "text/plain", Aws::Map<Aws::String, Aws::String>());
			}

			//cout << "\r Please wait .. uploading to aws" << endl;
			//requestPtr->WaitUntilFinished();
			while (requestPtr->GetStatus() != TransferStatus::COMPLETED)
			{
				pUploader->progress = int(100 * (requestPtr->GetBytesTransferred() / (double)requestPtr->GetBytesTotalSize()));
				//cout << endl << " AWS Upload of" << filename << " Progress : " << int(100 * (requestPtr->GetBytesTransferred() / (double)requestPtr->GetBytesTotalSize()))<< "%" << endl;

				//cout << "\r Finished uploading" << endl;
				//int v1 = rand() % 100; // to avoid simultaneous update from multi threads
				this_thread::sleep_for(std::chrono::milliseconds(5000));

				if (requestPtr->GetStatus() == TransferStatus::FAILED)
				{
					if (Settings::GetInstance().GetNetworkDown()) // If NW down
					{
						// We need to save the file full path for continue later
						if (pUploader->upload_pending) // upload pending
						{
							CompressorUtils::WriteAWSPendingItems(pUploader->raw_with_extn, pUploader->in_path_full, pUploader->aws_path_full);
						}
						else
						{
							CompressorUtils::WriteAWSPendingItems(pUploader->raw_with_extn, pUploader->file_path.c_str(), folder_path);
						}
						break;
					}
					else if ( AWS_RETRY_COUNT < pUploader->retryCount )
					{
						pUploader->retryCount++;
						AWSUploader::aws_thread(pUploader);
						break;
					}
					else
					{
						//log error
						LOG(pUploader->uploader->m_Logger, LOG_ERR, "Failed to upload --->" + folder_path);
						pUploader->progress = 100;
						break;
					}
				}

				if (pUploader->uploader->m_Shutdown)
				{
					return;
				}
			} // end of while


			pUploader->status = AWS_Free;
			pUploader->progress = 0;

			if (!pUploader->btext)
			{
				pUploader->uploader->UpdateTotalCount();
			}

			// clearing the uploaded file
			if (Settings::GetInstance().ClearTempFolder())
			{
				remove(pUploader->file_path.c_str());
			}
		}
		catch(...)
		{
			LOG(pUploader->uploader->m_Logger, LOG_ERR, "Unexpected error in AWSUploader::aws_thread");
		}

		LOG(pUploader->uploader->m_Logger, LOG_DEBUG, "Exiting AWSUploader::aws_thread");
		return;
		//cout << "\r Failed to upload" << endl;
	}

	void AWSUploader::UpdateTotalCount()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_nFinished++;
	}

	void AWSUploader::CloseAws()
	{
		LOG(m_Logger, LOG_DEBUG, "Entering AWSUploader::CloseAws");
		Aws::ShutdownAPI(g_options);
		LOG(m_Logger, LOG_DEBUG, "Exiting AWSUploader::CloseAws");
	}
}