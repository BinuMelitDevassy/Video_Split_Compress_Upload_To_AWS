// Logger support
// Binu
// v2.0 06-2017 

#pragma once
#include "Commondef.h"
#include <string>
#include <fstream>

using namespace std;

namespace Id_Comp
{
	//Logger class with different levels of logging
	class Comp_Logger
	{
	public:


		Comp_Logger();
		void Init(int level, string filename);
		~Comp_Logger();
		void Log(Logger_Level nLevel, string msg);

	private:
		
		string GetFileName(bool init = false);

		int			m_maxLines;
		int			m_lines;
		int			m_level;
		ofstream	m_File;
		string		m_outFilename;
		int			m_nFileNum;
	};

	static inline void LOG(Comp_Logger* logger, Id_Comp::Logger_Level level, string message)
	{
		if (logger != nullptr)
		{
			logger->Log(level, message);
		}
	}
}

