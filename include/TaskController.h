// Task Controller, interace
// Binu
// v.1 05-2017 


#pragma once

#include "Commondef.h"
#include <string>
using namespace std;

//forward  declerations
namespace Id_Comp
{
	class DecoderBase;
	class EncoderBase;
	class DataQueue;
	class AWSUploader;
	class Comp_Logger;
}

#if defined(TASKCONTROLLER_EXPORT)
#define TASKCONTROLLER_API   __declspec(dllexport)
#else
#define TASKCONTROLLER_API   __declspec(dllimport)
#endif

namespace Id_Comp
{
	//This class will controller the entire process of decoding,
	//encoding and AWS uploading
	class TASKCONTROLLER_API TaskController
	{
	public:
		TaskController( CODEC_TYPE Codec_type_i );
		~TaskController();
		// Convert and upload
		bool Process( string in_file, string out_folder,
			string client_name, string camera_num, bool ignore_ts );
		bool Upload_chunk(string in_rawfile, string in_file, string out_aws_path);
		int GetCompressProgress();
		int GetUploaderProgress();
		void SetStartTime(int YY, int MM, int DD, int HH, int MN, int SS);
		void SetEndTime(int YY, int MM, int DD, int HH, int MN, int SS);
		void Close();
		void SetNetworkDown(bool Down);

	private:
		DecoderBase*	m_pDecoder;
		EncoderBase*	m_pEncoder;
		DataQueue*		m_pData;
		AWSUploader*	m_pAWSUploader;
		bool			m_bstarted;
		bool			m_bFinished;
		bool			m_bError;
		bool			m_InitDone; // for pending data upload
		Comp_Logger*	m_Logger;
		bool			m_Shutdown;
	};
}

