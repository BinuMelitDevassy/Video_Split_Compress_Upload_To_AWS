// Logger support
// Binu
// v2.0 06-2017 

#include "Comp_Logger.h"
#include "Settings.h"
#include <chrono>
#include <Windows.h>
#include <direct.h>

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S"
#define LOGNAME_SIZE 20


namespace Id_Comp
{

	Comp_Logger::Comp_Logger():
		m_maxLines( 10000 ), // need to make this configurable
		m_lines(0),
		m_nFileNum(0)
	{
	}

	void Comp_Logger::Init(int level, string filepath)
	{
		// Extracting filename
		string filename = filepath;
		const size_t last_slash_idx = filename.find_last_of("\\/");
		if (std::string::npos != last_slash_idx)
		{
			filename.erase(0, last_slash_idx + 1);
		}
		m_outFilename = filename;
		m_level = level;

		m_File.open(GetFileName( true), ofstream::out);
		m_lines = 0;
	}

	void Comp_Logger::Log(Logger_Level nLevel, string msg)
	{
		if (m_maxLines < m_lines)
		{
			m_File.close();
			m_File.open(GetFileName(), ofstream::out);
			m_lines = 0;
		}
		
		//checking log level
		if (nLevel >= m_level)
		{
			static char date_time[LOGNAME_SIZE];
			time_t now = time(0);
			strftime(date_time, sizeof(date_time), LOGNAME_FORMAT, localtime(&now));
			m_File << date_time<<" -----> "<<msg << endl;
			m_lines++;
		}
	}

	Comp_Logger::~Comp_Logger()
	{
		m_File.close();
	}

	
	int DeleteDirectory(const std::string &refcstrRootDirectory,
		bool              bDeleteSubdirectories = true)
	{
		bool            bSubdirectory = false;       // Flag, indicating whether
													 // subdirectories have been found
		HANDLE          hFile;                       // Handle to directory
		std::string     strFilePath;                 // Filepath
		std::string     strPattern;                  // Pattern
		WIN32_FIND_DATAA FileInformation;             // File information


		strPattern = refcstrRootDirectory + "\\*.*";
		hFile = ::FindFirstFileA(strPattern.c_str(), &FileInformation);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (FileInformation.cFileName[0] != '.')
				{
					strFilePath.erase();
					strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

					if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (bDeleteSubdirectories)
						{
							// Delete subdirectory
							int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
							if (iRC)
								return iRC;
						}
						else
							bSubdirectory = true;
					}
					else
					{
						// Set file attributes
						if (::SetFileAttributesA(strFilePath.c_str(),
							FILE_ATTRIBUTE_NORMAL) == FALSE)
							return ::GetLastError();

						// Delete file
						if (::DeleteFileA(strFilePath.c_str()) == FALSE)
							return ::GetLastError();
					}
				}
			} while (::FindNextFileA(hFile, &FileInformation) == TRUE);

			// Close handle
			::FindClose(hFile);

			DWORD dwError = ::GetLastError();
			if (dwError != ERROR_NO_MORE_FILES)
				return dwError;
			else
			{
				if (!bSubdirectory)
				{
					// Set directory attributes
					if (::SetFileAttributesA(refcstrRootDirectory.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete directory
					if (::RemoveDirectoryA(refcstrRootDirectory.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		}

		return 0;
	}

	string Comp_Logger::GetFileName(bool init)
	{
		string filename;
		string OutputFolder = "Logs";

		if (Settings::GetInstance().ClearLog() && init)
		{
			DeleteDirectory(OutputFolder);
		}

		//Check folder exists, if not create it
		if (CreateDirectoryA(OutputFolder.c_str(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError())
		{
			// done
		}
		else
		{
			// Failed to create directory.
		}

		string folder = "Logs\\";
		string num;
		sprintf((char*)num.c_str(), "%d", m_nFileNum);
		filename = folder.c_str() + m_outFilename + "_" + num.c_str() + ".log";;
		return filename;
	}

}
