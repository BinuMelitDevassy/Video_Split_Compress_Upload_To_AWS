// Common defnitions
// Binu
// v1.0 05-2017 

#pragma once
#include <string>

using namespace std;

//This file holds the common defnitions
namespace Id_Comp
{
	static void avlog_cb(void *, int level, const char * szFmt, va_list varg) {
		//do nothing...
	}

	enum class CODEC_TYPE
	{
		FFMPEG,
		WMI
	};

	//logger enums
	enum  Logger_Level
	{
		LOG_DEBUG = 1, 
		LOG_INFO,
		LOG_WARN,
		LOG_ERR,
		LOG_NONE
	};
	//Encoder enums
	typedef enum Frame_Status
	{
		FRAME_BUFF_READY,
		FRAME_READ_READY,
		FRAME_WRITE_READY,
		FRAME_NOT_READY
	}Frame_Status;

	typedef enum File_Status
	{
		FILE_PROCESSING,
		FILE_FINISHED,
		FILE_NOTSTARTED
	}File_Status;

	const int AWS_RETRY_COUNT = 20;

	const string g_pendingLists("aws_pending.txt");

	//Config strings
	const string g_configfile(".\\uploader_config.ini");
	const string g_config_compressor("compressor_settings");
	const string g_config_aws("aws_settings");
	const string g_config_skip("frames_skip");
	const string g_config_keep("frames_keep");
	const string g_config_resize("resize");
	const string g_config_encoder("encoder");
	const string g_config_bitrate("bit_rate");
	const string g_config_splitlength("split_length");
	const string g_config_rejectlow("reject_low");
	const string g_config_rejectfps("reject_fps");
	const string g_config_awsupload("aws_upload");
	const string g_config_awsbucket("aws_bucket");
	const string g_config_awsregion("aws_region");
	const string g_config_awsthreads("aws_threads");
	const string g_config_time_stamp("time_stamp");
	const string g_config_temp_folder("temp");
	const string g_config_clear_temp("temp_clear");
	const string g_config_fps_low("fps_low");
	const string g_config_fps_high("fps_high");
	const string g_config_clear_log("log_clear");
	const string g_config_log_level("log_level");
}
