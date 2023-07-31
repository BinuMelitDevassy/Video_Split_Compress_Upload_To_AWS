// WMI Encoder
// Binu
// v1.0 05-2017 

#pragma once

#include "EncoderBase.h"

namespace Id_Comp
{
	// class implements the Windows media interface for encoder
	// To implement later
	class WMIEncoder : public EncoderBase
	{
	public:
		WMIEncoder();
		~WMIEncoder();

		virtual bool Initialize(DataQueue* data_i, int fps,
			int width, int height, int bitrate, int video_length_s,
			int buffer_size, Comp_Logger* Logger) {
			return true;
		};
		virtual bool Run(std::string out_path, int encoder) { return true; };
		virtual int GetCurrentFrame() { return 0; };
		virtual File_Status GetStatus() { return FILE_NOTSTARTED; };
		virtual void Wait_UntilFinish() {};
		virtual void close() {};
		virtual float GetDuration() { return 0; };
		virtual void ShtdownAll() {};
	};
}

