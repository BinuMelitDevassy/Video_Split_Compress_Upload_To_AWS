// Data queue
// Binu
// v.1 05-2017 

#pragma once

#include "FrameData.h"
#include "Commondef.h"
#include <mutex>

using namespace std;

namespace Id_Comp
{
	//Holding the data queue between decoding and encoding
	class DataQueue
	{
	public:

		DataQueue();
		~DataQueue();

		FrameData* GetWriteBuffer();
		FrameData* GetReadBuffer();

		File_Status GetFileStatus() {
			return m_FileStatus;
		};

		void SetFileStatus(File_Status status_i) {
			m_FileStatus = status_i;
		};

		void SetPrevPos(int pos_i) {
			m_nCurrentBuff = pos_i % FRAME_BUFFER_SIZE;
		};

		void InitializeDataFrames(uint8_t* buffer, int width, int height, int fps);

		FrameData* GetPrevBuffer();
		int FRAME_BUFFER_SIZE = 150; //2 sec default
		
	private:

		FrameData** m_pFrameDatas;
		File_Status m_FileStatus;
		
		int				m_nCurrentBuff = 0;
		int				m_lastframe = 0;
	};
}
    
