// Utility functions, general functions
// Binu
// v.0 04-2017 

#pragma once
#include <string>
#include <list>
#include <vector>

using namespace std;

// This class will handle the common utility functions
static class CompressorUtils
{
public:

	CompressorUtils();
	~CompressorUtils();
	static string getTimeStampedFileNamev1(const std::string& aFileName, int duration,
										int count, int skip, bool ignore_ts,
										bool last_frame = false, int last_dur = 0);
	static string getTimeStampedFileNamev2(const std::string& aFileName, int duration,
		int count, int skip, int fkeep, bool ignore_ts,
		bool last_frame = false, int last_dur = 0);
	static string CreateOutFolder(string out_folder, string client, string camera);
	static void ClearOutFolder(string out_folder);
	static string GetCurrDateandTime();
	static void WriteAWSPendingItems( string filename_full, string clip_path, string aws_path );
	static list<string> GetAWSPendingItems( string file_path);
	static std::vector<std::string> split(const string input, char delim);
};

