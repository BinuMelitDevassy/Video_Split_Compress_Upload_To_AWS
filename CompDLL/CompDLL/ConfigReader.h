// Configuaration reader
// Binu
// v.0 04-2017 

#pragma once

#include <string>

using namespace std;

namespace Id_Comp
{
	// Reads configaration values from ini file
	class ConfigReader
	{
	public:
		ConfigReader(string szFileName);
		~ConfigReader();

		int		ReadInteger( string szSection, string szKey, int iDefaultValue );
		float	ReadFloat( string szSection, string szKey, float fltDefaultValue );
		bool	ReadBoolean( string szSection, string szKey, bool bolDefaultValue );
		string	ReadString( string szSection, string szKey, string szDefaultValue );

	private:
		string m_szFileName;
	};
}

