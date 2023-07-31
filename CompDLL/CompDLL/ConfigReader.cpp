// Configuaration reader
// Binu
// v.0 04-2017 

#include "ConfigReader.h"
#include <iostream>
#include <Windows.h>

namespace Id_Comp
{

	ConfigReader::ConfigReader(string szFileName)
	{
		m_szFileName = szFileName;
	}


	ConfigReader::~ConfigReader()
	{
	}

	int ConfigReader::ReadInteger(string szSection, string szKey, int iDefaultValue)
	{
		int iResult = GetPrivateProfileIntA(szSection.c_str(),
					  szKey.c_str(), iDefaultValue, m_szFileName.c_str());
		return iResult;
	}

	float ConfigReader::ReadFloat(string szSection, string szKey, float fltDefaultValue)
	{
		string szResult;
		string szDefault;
		float fltResult;
		wsprintf((LPWSTR)szDefault.c_str(), L"%f", fltDefaultValue);
		GetPrivateProfileStringA(szSection.c_str(), szKey.c_str(),
								szDefault.c_str(), (LPSTR)szResult.c_str(), 255,
								m_szFileName.c_str());
		fltResult = atof(szResult.c_str());
		return fltResult;
	}

	bool ConfigReader::ReadBoolean(string szSection, string szKey, bool bolDefaultValue)
	{
		string szResult;
		string szDefault;;
		bool bolResult;
		wsprintf((LPWSTR)szDefault.c_str(), L"%s", bolDefaultValue ? L"True" : L"False");
		GetPrivateProfileStringA(szSection.c_str(), szKey.c_str(), szDefault.c_str(),
								(LPSTR)szResult.c_str(), 255, m_szFileName.c_str());
		bolResult = (strcmp(szResult.c_str(), "True") == 0 ||
					 strcmp(szResult.c_str(), "true") == 0) ? true : false;
		return bolResult;
	}

	string ConfigReader::ReadString(string szSection, string szKey, const string szDefaultValue)
	{
		string szResult("", 255);
		GetPrivateProfileStringA(szSection.c_str(), szKey.c_str(),
			szDefaultValue.c_str(), (LPSTR)szResult.c_str(), 255, m_szFileName.c_str());
		string res(szResult.c_str());
		return res;
	}
}
