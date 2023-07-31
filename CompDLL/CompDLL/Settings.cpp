// Settings holder
// Binu
// v.1 05-2017 

#include "Settings.h"
#include "Commondef.h"

namespace Id_Comp
{
	Settings& Settings::GetInstance()
	{
		// Since it's a static variable, if the class has already been created,
		// It won't be created again.
		// And it **is** thread-safe in C++11.

		static Settings settings;

		// Return a reference to our instance.
		return settings;
	}

	Settings::Settings():
		m_NWDown( false )
	{
		m_pConfig = new ConfigReader(g_configfile);
	}


	Settings::~Settings()
	{
		if (m_pConfig) delete m_pConfig;
	}

	void Settings::SetNetworkDown(bool Down)
	{
		m_NWDown = Down;
	}

	bool Settings::GetNetworkDown()
	{
		return m_NWDown;
	}

	int Settings::GetFrameSkip()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_skip, 0);
	}

	int Settings::GetFrameKeep()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_keep, 0);
	}

	int Settings::GetResize()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_resize, 100);
	}

	int Settings::GetEncoder()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_encoder, 1);
	}

	int Settings::GetBitrate()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_bitrate, 4000);
	}

	int Settings::GetSplitLength()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_splitlength, 10);
	}

	int Settings::GetAwsThreads()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_awsthreads, 5);
	}

	int Settings::GetMinFPS()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_fps_low, 10);
	}

	int Settings::GetMaxFPS()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_fps_high, 30);
	}

	int Settings::GetLogLevel()
	{
		return m_pConfig->ReadInteger(g_config_compressor, g_config_log_level, 3);
	}

	bool Settings::GetRejectlow()
	{
		return m_pConfig->ReadBoolean(g_config_compressor, g_config_rejectlow, true);
	}

	bool Settings::GetAwsUpload()
	{
		return m_pConfig->ReadBoolean(g_config_compressor, g_config_awsupload, true);
	}

	bool Settings::ClearTempFolder()
	{
		return m_pConfig->ReadBoolean(g_config_compressor, g_config_clear_temp, true);
	}
	
	bool Settings::ClearLog()
	{
		return m_pConfig->ReadBoolean(g_config_compressor, g_config_clear_log, true);
	}

	bool Settings::GetRejectFps()
	{
		return m_pConfig->ReadBoolean(g_config_compressor, g_config_rejectfps, true);
	}

	string Settings::GetTimestamp()
	{
		return m_pConfig->ReadString(g_config_compressor, g_config_time_stamp, "???????YYYYMMDDHHMISS");
	}

	string Settings::GetTempFolder()
	{
		return m_pConfig->ReadString(g_config_compressor, g_config_temp_folder, "Temp");
	}

	void Settings::SetStartTime(int YY, int MM, int DD, int HH, int MN, int SS)
	{
		m_StartTime.YY = YY;
		m_StartTime.MM = MM;
		m_StartTime.DD = DD;
		m_StartTime.HH = HH;
		m_StartTime.MN = MN;
		m_StartTime.SS = SS;
	}

	void Settings::SetEndTime(int YY, int MM, int DD, int HH, int MN, int SS)
	{
		m_EndTime.YY = YY;
		m_EndTime.MM = MM;
		m_EndTime.DD = DD;
		m_EndTime.HH = HH;
		m_EndTime.MN = MN;
		m_EndTime.SS = SS;
	}

	Comp_Time& Settings::GetStartTime()
	{
		return m_StartTime;
	}

	Comp_Time& Settings::GetEndTime()
	{
		return m_EndTime;
	}

	string Settings::GetCurTime()
	{
		return m_curtime;
	}
	void Settings::SetCurTime(string cur_time)
	{
		m_curtime = cur_time;
	}
}
