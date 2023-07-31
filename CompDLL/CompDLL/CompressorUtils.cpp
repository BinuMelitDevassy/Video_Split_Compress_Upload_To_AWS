// Utility functions, general functions
// Binu
// v.0 04-2017 

#include "CompressorUtils.h"
#include "Settings.h"
#include "Commondef.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <regex>
#include <Windows.h>

using namespace Id_Comp;

CompressorUtils::CompressorUtils()
{
}


CompressorUtils::~CompressorUtils()
{
}

string CompressorUtils::getTimeStampedFileNamev1(const std::string& aFileName,
						int duration, int count, int skip, bool ignore_ts,
						bool last_frame, int last_dur )
{
	// Remove directory if present.
	string filename = aFileName;
	const size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	int totalskip = 0;
	string hour = "0";
	string min = "0";
	string sec = "0";
	string new_fileName = filename;
	if (!ignore_ts) // have time stamp
	{
		//counting ?
		int iQuestions = 0;
		string timestamp = Id_Comp::Settings::GetInstance().GetTimestamp();
		for (int iLoop = 0; iLoop < timestamp.length(); iLoop++)
			if (timestamp[iLoop] == '?')
				iQuestions++;
		totalskip = iQuestions + 4 + 2 + 2; // year + month + date

		string hour = filename.substr(totalskip, 2);
		string min = filename.substr(totalskip + 2, 2);
		string sec = filename.substr(totalskip + 4, 2);
		new_fileName = filename.substr(0, totalskip);
	}
	else
	{
		new_fileName = filename.substr(0, filename.size() - 4); // remove extn
		new_fileName = new_fileName + "_";
	}

	string ext = ".mp4";


	int total_time = (duration * count * (skip+1) ) / 60;
	//int s = total_time % 10; // single
	//int t = total_time / 10 % 10; // tens
	//int h = total_time / 100 % 10; // hounderds
	//int u = total_time / 100 % 10; // thousands

	int nh = atoi(hour.c_str());
	int nm = atoi(min.c_str()) + total_time;
	int ns = atoi(sec.c_str());

	if (nm > 59)
	{
		nh++;
		nm = 0;
	}

	if (last_frame)
	{
		duration = (int)last_dur;
	}

	stringstream newFileName;
	newFileName << new_fileName << setfill('0')<< setw(2)<<nh << setfill('0')<<
		setw(2)<<nm << setfill('0')<< setw(2)<< ns << "_" << duration << "_FS" << skip<<ext;

	return newFileName.str();
}

string CompressorUtils::getTimeStampedFileNamev2(const std::string& aFileName,
	int duration, int count, int skip, int fkeep, bool ignore_ts,
	bool last_frame, int last_dur)
{
	// Remove directory if present.
	string filename = aFileName;
	const size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	int totalskip = 0;
	
	string new_fileName = filename;
	new_fileName = filename.substr(0, filename.size() - 4); // remove extn
	new_fileName = new_fileName + "_";
	

	string ext = ".mp4";

	if (last_frame)
	{
		duration = (int)last_dur;
	}

	stringstream newFileName;
	newFileName << "_"<< setw(2)<< setfill('0') <<count <<"_"<< new_fileName << setw(3) << setfill('0') << duration << "_K"<<fkeep << "S"<< skip << ext;

	return newFileName.str();
}

string CompressorUtils::CreateOutFolder(string out_folder, string client, string camera)
{
	//Check folder exists, if not create it
	if (CreateDirectoryA(out_folder.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		// done
	}
	out_folder = out_folder + "\\" + client;
	if (CreateDirectoryA(out_folder.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		// done
	}
	out_folder = out_folder +  "\\" + camera;

	if (CreateDirectoryA(out_folder.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		// done
	}
	return out_folder;
}

void CompressorUtils::ClearOutFolder(string out_folder)
{
	string parent = "..\\" + out_folder;
	RemoveDirectoryA(out_folder.c_str());
}

string CompressorUtils::GetCurrDateandTime()
{
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	std::ostringstream oss;
	oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
	string dat = oss.str();
	return dat;
}

void CompressorUtils::WriteAWSPendingItems(string filename_full, string clip_path, string aws_path)
{
	static std::mutex lok;
	std::lock_guard<std::mutex> lock(lok);
	std::ofstream out(g_pendingLists, std::ofstream::out | std::ofstream::app );
	out << filename_full + "*" + clip_path + "*" + aws_path << endl;
	out.close();
}

list<string> CompressorUtils::GetAWSPendingItems(string file_path)
{
	std::ifstream file(file_path);
	std::string str;
	list<string> pending_items;
	while (std::getline(file, str))
	{
		pending_items.push_back(str);
	}

	file.close();
	// clear file
	std::remove(file_path.c_str());
	//DeleteFileA(file_path.c_str());

	return pending_items;
}

std::vector<std::string> CompressorUtils::split(const string input, char delim)
{
	std::vector<std::string> cont;
	std::stringstream ss(input);
	std::string token;
	while (std::getline(ss, token, delim)) {
		cont.push_back(token);
	}
	return cont;
}