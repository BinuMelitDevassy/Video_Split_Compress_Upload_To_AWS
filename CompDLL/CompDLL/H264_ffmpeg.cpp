// H264 Encoder
// Binu
// v.1 05-2017 

// Class core logic is taken from ffmpeg sample code
// More details check their code

#include "H264_ffmpeg.h"


H264_ffmpeg::H264_ffmpeg(Comp_Logger* Logger):
	m_Logger( Logger)
{
	LOG(m_Logger, LOG_DEBUG, "Entering H264_ffmpeg::H264_ffmpeg");
	m_filename = new char [1024];
	m_AVIMutex = false;
	LOG(m_Logger, LOG_DEBUG, "Exiting H264_ffmpeg::H264_ffmpeg");
}

H264_ffmpeg::~H264_ffmpeg()
{
	LOG(m_Logger, LOG_DEBUG, "Entering H264_ffmpeg::~H264_ffmpeg");
	delete [] m_filename;
	LOG(m_Logger, LOG_DEBUG, "Exiting H264_ffmpeg::~H264_ffmpeg");
}


// Sets up the video codec
bool H264_ffmpeg::SetupCodec(const char *filename, AVCodecID codec_id)
{
	LOG(m_Logger, LOG_DEBUG, "Exiting H264_ffmpeg::SetupCodec()");
	if (m_AVIMutex) {return false;}
	
	//Some inits
	int ret;
	m_sws_flags = SWS_BICUBIC;
	m_frame_count=0;
	
	//You must call these subroutines otherwise you get errors!!
	avcodec_register_all();
	av_register_all();
	
	//allocate the output media context
	avformat_alloc_output_context2(&m_oc, NULL, NULL, filename);
	if (!m_oc) {
		//if context failed check by specifying container
		avformat_alloc_output_context2(&m_oc, NULL, "avi", filename);
	}
	if (!m_oc) {
		//failed
		LOG(m_Logger, LOG_ERR, "Error - failed to create media context -Exiting H264_ffmpeg::SetupCodec");
		return false;
	}
	
	//Get the format determined by the conatiner
	m_fmt = m_oc->oformat;

	// Add the audio and video streams using the default format codecs
	// and initialize the codecs.
	m_video_st = NULL;

	//Setup the codecs
	//AV_CODEC_ID_H264 or V_CODEC_ID_MPEG2VIDEO;
	m_fmt->video_codec = codec_id;
	m_fmt->audio_codec = AV_CODEC_ID_NONE;
	

	// Add an output stream.
	{
		//Init AV Stream
		AVStream *st;

		//find the encoder
		m_video_codec = avcodec_find_encoder(m_fmt->video_codec);
		if (!(m_video_codec))
		{

			LOG(m_Logger, LOG_ERR, "Error - failed to create codec -Exiting H264_ffmpeg::SetupCodec");
			return false;
		}

		//Create new video stream
		st = avformat_new_stream(m_oc, m_video_codec);
		if (!st)
		{
			LOG(m_Logger, LOG_ERR, "Error - failed to create output stream -Exiting H264_ffmpeg::SetupCodec");
			return false;
		}
		st->id = m_oc->nb_streams-1;

		//Set codec context
		m_c = st->codec;
		
		//Setup fundumental video stream parameters
		m_c->codec_id = m_fmt->video_codec;
		m_c->bit_rate = m_AVIMOV_BPS;			//Bits Per Second 
		m_c->width    = m_AVIMOV_WIDTH;			//Note Resolution must be a multiple of 2!!
		m_c->height   = m_AVIMOV_HEIGHT;		//Note Resolution must be a multiple of 2!!
		m_c->time_base.den = m_AVIMOV_FPS;		//Frames per second
		m_c->time_base.num = 1;
		m_c->gop_size      = m_AVIMOV_GOB;		// Intra frames per x P frames
		m_c->pix_fmt       = AV_PIX_FMT_YUV420P;//Do not change this, H264 needs YUV format not RGB


		//Some formats want stream headers to be separate.
		if (m_oc->oformat->flags & AVFMT_GLOBALHEADER)
			m_c->flags |= CODEC_FLAG_GLOBAL_HEADER;

		//Set our video stream pointer
		m_video_st=st;

		
	}

	
	// Now that all the parameters are set, we can open the audio and
	// video codecs and allocate the necessary encode buffers.
	{
		//Allocated Codec Context
		AVCodecContext *c = m_video_st->codec;

		//Open the codec
		ret = avcodec_open2(c, m_video_codec, NULL);
		if (ret < 0) 
		{
			LOG(m_Logger, LOG_ERR, "Error - failed to open codec -Exiting H264_ffmpeg::SetupCodec");
			return false;
		}

		//allocate and init a re-usable frame
		m_frame = av_frame_alloc();
		if (!m_frame)
		{
			LOG(m_Logger, LOG_ERR, "Error - failed to create frame -Exiting H264_ffmpeg::SetupCodec");
			return false;
		}
	}

	//Tell FFMPEG that we are going to write encoded frames to a file
	av_dump_format(m_oc, 0, filename, 1);

	//open the output file, if needed
	if (!(m_fmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&m_oc->pb, filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			LOG(m_Logger, LOG_ERR, "Error - failed to create ooutput file -Exiting H264_ffmpeg::SetupCodec");
			return false;
		}
	}


	//Write the stream header, if any.
	ret = avformat_write_header(m_oc, NULL);
			

	if (ret < 0) 
	{
		LOG(m_Logger, LOG_ERR, "Error - failed to write header -Exiting H264_ffmpeg::SetupCodec");
		return false;
	}
	
	//Set frame count to zero
	if (m_frame)
		m_frame->pts = 0;
	
	//Frame is initalised!
	m_AVIMutex=true;


	LOG(m_Logger, LOG_DEBUG, "Exiting H264_ffmpeg::H264_ffmpeg");
	return true;
}


// Writes a Video Frame that is stored in m_src_picture
void H264_ffmpeg::WriteFrame(AVFrame * RGBFrame){

	LOG(m_Logger, LOG_DEBUG, "Entering H264_ffmpeg::WriteFrame");
	//If video is not initalised then don't write frame
	if (!m_AVIMutex) 
	{
		LOG(m_Logger, LOG_INFO, "Frame miss H264_ffmpeg::WriteFrame");
		return;
	}

	//Calculate video time	
	m_video_time = m_video_st ? m_video_st->pts.val * av_q2d(m_video_st->time_base) : 0.0;

	////Inits
	int ret;
	//static struct SwsContext *sws_ctx;
	AVCodecContext *c = m_video_st->codec;

	//Some inits for encoding the frame
	AVPacket pkt = { 0 };
	int got_packet;
	av_init_packet(&pkt);

	//Encode the frame
	RGBFrame->pts = m_frame->pts;
	ret = avcodec_encode_video2(c, &pkt, RGBFrame, &got_packet);
	if (ret < 0) 
	{
		LOG(m_Logger, LOG_INFO, "Frame miss H264_ffmpeg::WriteFrame");
		return;
	}
	
	//If size of encoded frame is zero, it means the image was buffered.
	if (!ret && got_packet && pkt.size) {
		pkt.stream_index = m_video_st->index;
		
		//Write the compressed frame to the media file.
		ret = av_interleaved_write_frame(m_oc, &pkt);

		//if non-zero then it means that there was something wrong writing the frame to
		//the file
		if (ret != 0) 
		{
			LOG(m_Logger, LOG_INFO, "Frame miss H264_ffmpeg::WriteFrame");
			return;
		}
	} 
	else {
		ret = 0;
	}
	
	//Increment Frame counter
	m_frame_count++;
	m_frame->pts += av_rescale_q(1, m_video_st->codec->time_base, m_video_st->time_base);

	LOG(m_Logger, LOG_DEBUG, "Exiting H264_ffmpeg::WriteFrame");
}



//=============================
// Setup Video
//-----------------------------
// Sets up the Video and Opens the Video File
//-----------------------------
bool H264_ffmpeg::SetupVideo(char * filename, int Width, int Height,
							 int FPS, int GOB, int BitPerSecond, AVCodecID codec_id )
{
	LOG(m_Logger, LOG_DEBUG, "Entering H264_ffmpeg::SetupVideo");

	//Copy filename to local string
	sprintf(m_filename,filename);
	
	//Set movie parameters
	m_AVIMOV_WIDTH=Width;	//Movie width
	m_AVIMOV_HEIGHT=Height;	//Movie height
	m_AVIMOV_FPS=FPS;		//Movie frames per second
	m_AVIMOV_GOB=GOB;		//I frames per no of P frames, see note below!
	m_AVIMOV_BPS=BitPerSecond; //Bits per second, if this is too low then movie will become garbled
	
	LOG(m_Logger, LOG_DEBUG, "Exiting H264_ffmpeg::SetupVideo");

	return SetupCodec(m_filename, codec_id);
	
	//------------
	//NOTE: on GOB
	//------------
	//An I frame is an entire frame stored. A P frame is only a partial frame that is 
	//encoded based on the previous frame. 
	//
	//Think of I frames as refreshing the entire video, and P frames as just encoding the 
	//moving bits of the frame.
	//
	//A GOB of 10 means that for every 10 P frames there is only 1 I frame, i.e. the frame is
	//only 'refreshed' every 10 frames.
	//------------
}


// Close Video Codec
void H264_ffmpeg::CloseCodec(void){

	LOG(m_Logger, LOG_DEBUG, "Entering H264_ffmpeg::CloseCodec");
	//If video is not initalised then don't close frame
	if (!m_AVIMutex) {return;}

	//Write trailing bits
	av_write_trailer(m_oc);

	//Close Video codec
    avcodec_close(m_video_st->codec);

	//Free our frames
	try
	{
		av_free(m_frame);
	}
	catch(...)
	{ }

	//A bit more cleaning
    if (!(m_fmt->flags & AVFMT_NOFILE))
        avio_close(m_oc->pb);

    avformat_free_context(m_oc);

	//Set open flag clear
	m_AVIMutex=false;
	LOG(m_Logger, LOG_DEBUG, "Exiting H264_ffmpeg::CloseCodec");
}


// Close Video Codec
void H264_ffmpeg::CloseVideo(void){
	
	//Close Video Codec and write end of file
	CloseCodec();	
}
