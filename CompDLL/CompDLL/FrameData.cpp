// Frame Data
// Binu
// v.1 05-2017 

#include "FrameData.h"

extern "C" {
#include <libavcodec/avcodec.h>
}


namespace Id_Comp
{
	FrameData::FrameData()
	{
		// Allocate an AVFrame structure
		m_pFrameRGB = av_frame_alloc();
	}


	FrameData::~FrameData()
	{
		//releasing memory
		av_frame_free(&m_pFrameRGB);
	}

	void FrameData::InitFrame(unsigned char* buffer, int width, int height)
	{
		// Assign appropriate parts of buffer to image planes in pFrameRGB
		// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
		// of AVPicture
		avpicture_fill((AVPicture *)m_pFrameRGB, buffer, AV_PIX_FMT_YUV420P,
						width, height);
	}

	void FrameData::copyframe(FrameData* src)
	{
		av_frame_unref(m_pFrameRGB);
		av_frame_ref(m_pFrameRGB, src->m_pFrameRGB);
		//av_frame_copy(m_pFrameRGB, src->m_pFrameRGB);
		//m_pFrameRGB = av_frame_clone(src->m_pFrameRGB);
		/*m_pFrameRGB->format = src->m_pFrameRGB->format;
		m_pFrameRGB->width = src->m_pFrameRGB->width;
		m_pFrameRGB->height = src->m_pFrameRGB->height;
		m_pFrameRGB->channels = src->m_pFrameRGB->channels;
		m_pFrameRGB->channel_layout = src->m_pFrameRGB->channel_layout;
		m_pFrameRGB->nb_samples = src->m_pFrameRGB->nb_samples;
		av_frame_get_buffer(m_pFrameRGB, 32);
		av_frame_copy(m_pFrameRGB, src->m_pFrameRGB);
		av_frame_copy_props(m_pFrameRGB, src->m_pFrameRGB);*/
	}
}
