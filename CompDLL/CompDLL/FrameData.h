// Frame Data
// Binu
// v.1 05-2017 

#pragma once

#include "Commondef.h"

//Forward decleration
struct AVFrame;

namespace Id_Comp
{
	//This class will hold the frame data between decoder and encoder
	class FrameData
	{
	public:
		FrameData();
		~FrameData();
		
		//Init Frame
		void InitFrame( unsigned char* buffer, int width, int height);
		
		//Get data pointer for read or write
		AVFrame*& GetData() { return m_pFrameRGB; };
		AVFrame*  GetData1() { return m_pFrameRGB; };

		Frame_Status GetStatus() { return m_Status; };
		void SetStatus(Frame_Status status_i) { m_Status = status_i; };

		void copyframe(FrameData* src);

	private:
		AVFrame* m_pFrameRGB = nullptr;
		Frame_Status m_Status = FRAME_WRITE_READY;
	};
}

