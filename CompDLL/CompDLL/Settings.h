// Settings holder
// Binu
// v.1 05-2017 

#pragma once
#include "ConfigReader.h"

namespace Id_Comp
{
	// time struct
	struct Comp_Time 
	{
		int YY;
		int MM;
		int DD;
		int HH;
		int MN;
		int SS;
	};
	// This class will hold the  whole settings for the compressor
	class Settings
	{
	public:
		static Settings& GetInstance();
		~Settings();

		int GetFrameSkip();
		int GetFrameKeep();
		int GetResize();
		int GetEncoder();
		int GetBitrate();
		int GetSplitLength();
		int GetAwsThreads();
		int GetMinFPS();
		int GetMaxFPS();
		int GetLogLevel();
		bool GetRejectlow();
		bool GetRejectFps();
		bool GetAwsUpload();
		bool ClearTempFolder();
		bool ClearLog();
		string GetTimestamp();
		string GetTempFolder();

		void SetStartTime(int YY, int MM, int DD, int HH, int MN, int SS);
		void SetEndTime(int YY, int MM, int DD, int HH, int MN, int SS);
		Comp_Time& GetStartTime();
		Comp_Time& GetEndTime();

		string GetCurTime();
		void SetCurTime(string cur_time);
		void SetNetworkDown(bool Down);
		bool GetNetworkDown();

		// delete copy and move constructors and assign operators
		Settings(Settings const&) = delete;             // Copy construct
		Settings(Settings&&) = delete;                  // Move construct
		Settings& operator=(Settings const&) = delete;  // Copy assign
		Settings& operator=(Settings &&) = delete;      // Move assign

	private:
		//Restricting constructor
		Settings();
		ConfigReader* m_pConfig;
		Comp_Time m_StartTime;
		Comp_Time m_EndTime;
		string m_curtime;
		bool m_NWDown;
	};
}

