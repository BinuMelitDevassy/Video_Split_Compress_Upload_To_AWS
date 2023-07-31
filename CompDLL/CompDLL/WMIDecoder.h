// WMI Decoder
// Binu
// v1.0 05-2017 

#pragma once
#include "DecoderBase.h"

namespace Id_Comp
{
	// class implements the Windows media interface for decoder
	// To implement later
	class WMIDecoder : public DecoderBase
	{
	public:
		WMIDecoder();
		~WMIDecoder();
		virtual bool Initialize(int resize, DataQueue* data_i, std::string in_path, int framestoskip,
			int framestokeep, Comp_Logger* Logger) {
			return true;
		};
		virtual void Run() {};
		virtual int GetCurrentFrame() { return 0; };
		virtual File_Status GetStatus() {
			return FILE_NOTSTARTED;
		};
		virtual int GetProgress() { return 0; };
		virtual void ShtdownAll() {};
	};
}

