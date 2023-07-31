
// AWS Uploader class header
// Binu
// v1.0 05-2017 

#pragma once
#include "Commondef.h"
#include "Comp_Logger.h"
#include <aws/s3/S3Client.h>
#include <aws/transfer/TransferManager.h>
#include <aws/core/Aws.h>
#include <string>
#include <memory>

using namespace std;
using namespace Aws::Transfer;
namespace Id_Comp
{
	// This class will handle the AWS uploading of the compressed video
	class AWSUploader
	{

	public:

		AWSUploader( float nChunks, Comp_Logger* Logger );
		~AWSUploader();

		bool SetUpConnection();
		bool UploadVideo(string file_path, string client, string cam,
						 string out_folder, string raw_file_name, bool bText);
		void WaitToFinish();
		void CloseAws();
		int GetProgress();
		void ShtdownAll() { m_Shutdown = true; };
		bool UploadVideo(string raw_filename, string in_file, string out_aws_path);
	private:

		void UpdateTotalCount();
		
		enum AWS_Status
		{
			AWS_Free,
			AWS_Uploading,
			AWS_Error
		};

		typedef struct Uploader_Params
		{
			string raw_file_name = "";
			string raw_with_extn = "";
			string file_path = "";
			string client = "";
			string cam = "";
			string BucketName = "";
			Aws::Transfer::TransferManager* TrnsMgr = nullptr;
			AWS_Status status = AWS_Free;
			thread thread;
			shared_ptr<Aws::S3::S3Client> s3Client = nullptr;
			int progress = 0;
			AWSUploader *uploader = nullptr;
			bool btext = false;
			string out_folder = "";
			int retryCount = 0;
			//for pending upload
			string in_path_full = "";
			string aws_path_full = "";
			bool upload_pending = false;
		}Uploader_Params;

		static void aws_thread(Uploader_Params* pUploader); // the uploader thread

		Uploader_Params*					m_AwsThreads;
		string								m_sBucketName;
		string								m_sAccesKeyID;
		string								m_sSecretKey;
		int									m_nThreadCount;
		float							    m_fChunks;
		int									m_nFinished;
		std::mutex							m_mutex;
		Comp_Logger*						m_Logger;
		bool								m_Shutdown;
	};

}