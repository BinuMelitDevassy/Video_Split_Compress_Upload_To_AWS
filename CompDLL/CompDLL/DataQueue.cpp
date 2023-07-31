// Data queue
// Binu
// v.1 05-2017 

#include "DataQueue.h"

namespace Id_Comp
{

	DataQueue::DataQueue()
	{
		m_FileStatus = FILE_NOTSTARTED;
		//Initialize the data frames
		
	}

	   
	DataQueue::~DataQueue()
	{
		// Freeing up all memory
		for (int ncount = 0; ncount < FRAME_BUFFER_SIZE; ncount++)
		{
			delete m_pFrameDatas[ncount];
		}
		delete[] m_pFrameDatas;
	}

	void DataQueue::InitializeDataFrames(uint8_t* buffer, int width, int height, int fps )
	{
		FRAME_BUFFER_SIZE = FRAME_BUFFER_SIZE * fps;
		m_pFrameDatas = new FrameData*[FRAME_BUFFER_SIZE];
		for (int ncount = 0; ncount < FRAME_BUFFER_SIZE; ncount++)
		{
			m_pFrameDatas[ncount] = new FrameData();
		}
		// Initializing av frames
		for (int ncount = 0; ncount < FRAME_BUFFER_SIZE; ncount++)
		{
			m_pFrameDatas[ncount]->InitFrame( buffer, width, height );
		}
	}

	FrameData* DataQueue::GetReadBuffer()
	{
		// Getting read buffer
		for (int ncount = 0; ncount < FRAME_BUFFER_SIZE; ncount++)
		{
			if (m_pFrameDatas[ncount]->GetStatus() == FRAME_READ_READY)
			{
				m_pFrameDatas[ncount]->SetStatus(FRAME_NOT_READY);
				return 	m_pFrameDatas[ncount];
			}
		}
		return nullptr;
	}

	FrameData* DataQueue::GetPrevBuffer()
	{
		// Getting read buffer
		for (int ncount = 0; ncount < FRAME_BUFFER_SIZE; ncount++)
		{
			if (m_pFrameDatas[ncount]->GetStatus() == FRAME_BUFF_READY)
			{
				m_pFrameDatas[ncount]->SetStatus(FRAME_NOT_READY);
				//m_nCurrentBuff++;
				/*if (m_nCurrentBuff >= (FRAME_BUFFER_SIZE-1))
				{
					m_nCurrentBuff = 0;
				}*/
				return 	m_pFrameDatas[ncount];
			}		
		}
		return nullptr;
	}

	FrameData* DataQueue::GetWriteBuffer()
	{
		// Getting write buffer
		int ncount = m_lastframe;
		for (; ncount < FRAME_BUFFER_SIZE; ncount++)
		{
			if (m_pFrameDatas[ncount]->GetStatus() == FRAME_WRITE_READY)
			{
				m_pFrameDatas[ncount]->SetStatus(FRAME_NOT_READY);
				m_lastframe++;
				return 	m_pFrameDatas[ncount];
			}

		}
		if (ncount >= (FRAME_BUFFER_SIZE))
		{
			m_lastframe = 0;
		}
		return nullptr;
	}
}
