/*
 * OutputStream.h
 *
 *  Created on: 2015年2月11日
 *      Author: zz
 */

#ifndef FFOUTPUTSTREAM_H_
#define FFOUTPUTSTREAM_H_

#include "FFHeader.h"

class OutputStream
{
	enum {videoStreamIndex = 0, audioStreamIndex, avStreamNum};
public:
	OutputStream():m_fmtCtx(0), m_avstreamIndex( {0}), m_avstream( {0}) {}
	virtual ~OutputStream() {
		close();
	}

	void setUrl(const string& url) {
		m_url = url;
	}
	void setFormat(string& fmt) {
		m_format = fmt;
	}
	bool open();
	void close();

	bool writeFrame(Packet& pkt);

	void setStreamCodec(AVStream* audioStream, AVStream* videoStream) {
		m_avstreamIndex[videoStreamIndex] = 0;
		m_avstreamIndex[audioStreamIndex] = 1;

		m_avstream[videoStreamIndex] = videoStream;
		m_avstream[audioStreamIndex] = audioStream;
	}

	void setiFmtCtx(AVFormatContext* ifmtCtx) {
		m_ifmtCtx = ifmtCtx;
	}


	int av_copy_format_context(AVFormatContext* ofmt, AVFormatContext* ifmt) {
		m_avstream[videoStreamIndex] = avformat_new_stream(ofmt, NULL);
		m_avstream[audioStreamIndex] = avformat_new_stream(ofmt, NULL);
		
		for (unsigned int i = 0; i < ifmt->nb_streams; i++) {
			AVStream *istream = ifmt->streams[i];
			AVCodecContext *ictx = istream->codec;
			AVStream* ostream;
			if (ictx->codec_type == AVMEDIA_TYPE_AUDIO) {
				ostream = m_avstream[audioStreamIndex];
        	} else if (ictx->codec_type == AVMEDIA_TYPE_VIDEO) {
				ostream = m_avstream[videoStreamIndex];
        	}

			if (avcodec_copy_context(ostream->codec, istream->codec) < 0) {
				fprintf(stderr, "Could not open source file\n");
				return -1;
			}
			//ostream->time_base = istream->time_base;
			ostream->codec->codec_tag = 0;
			if (ofmt->oformat->flags & AVFMT_GLOBALHEADER)
				ostream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}

		return 0;
	}

	string getLastError() {
		return m_lastError;
	}
private:
	string m_url; // 输入地址
	string m_format; // 文件格式

	typedef map<string, string> Options;
	Options m_options; 	// 打开摄像头时设置的选项, 调用者需确保option可用

	string m_lastError;
	AVFormatContext* m_fmtCtx;

	AVFormatContext* m_ifmtCtx;

	int 		m_avstreamIndex[avStreamNum];
	AVStream* 	m_avstream[avStreamNum];
};

bool OutputStream::open()
{
	const char* fmt = m_format.empty() ? NULL : m_format.c_str();
	int error = avformat_alloc_output_context2(&m_fmtCtx, NULL, fmt, m_url.c_str());
	if (error<0) {
		LOGE("alloc %s failed\n", m_url.c_str());
		return false;
	}

	if (m_url.empty()) {
		m_lastError = "null url";
		return false;
	}

	// avformat_new_stream(m_fmtCtx, NULL);
	// avformat_new_stream(m_fmtCtx, NULL);

	// avcodec_copy_context(m_fmtCtx->streams[videoStreamIndex]->codec, m_avstream[videoStreamIndex]->codec);
	// avcodec_copy_context(m_fmtCtx->streams[audioStreamIndex]->codec, m_avstream[audioStreamIndex]->codec);

	av_copy_format_context(m_fmtCtx, m_ifmtCtx);
	if ((error = avio_open(&m_fmtCtx->pb, m_url.c_str(), AVIO_FLAG_WRITE)) < 0) {
		m_lastError = av_err2str(error);
		return false;
	}

	AVDictionary* options = NULL;
	for(Options::iterator it = m_options.begin(); it != m_options.end() && !m_options.empty(); ++it) {
		const char* key = it->first.c_str();
		const char* value = it->second.c_str();
		av_dict_set(&options, key, value, 0);
	}

	if ((error = avformat_write_header(m_fmtCtx, &options)) < 0) {
		LOGE("[%s] write header failed!\n", m_url.c_str()/*, get_error_text(error).c_str()*/);
		m_lastError = av_err2str(error);
		return false;
	}

	av_dump_format(m_fmtCtx, 0, m_url.c_str(), 1);
	return true;
}

bool OutputStream::writeFrame(Packet& pkt)
{
	AVPacket* c_pkt = pkt.c_pkt();
	// if (c_pkt.weight && c_pkt.height) {
	// c_pkt->stream_index = m_avstreamIndex[videoStreamIndex];
	// } else {
	// c_pkt->stream_index = m_avstreamIndex[audioStreamIndex];
	// }


	const AVRational rational = {1, 1000};
	c_pkt->pts = av_rescale_q(c_pkt->pts, rational, m_avstream[c_pkt->stream_index]->time_base);
	c_pkt->dts = av_rescale_q(c_pkt->dts, rational, m_avstream[c_pkt->stream_index]->time_base);

//	if(av_write_frame(m_fmtCtx, pkt->c_pkt()) < 0) {
	int ret = av_interleaved_write_frame(m_fmtCtx, pkt.c_pkt());
	if(ret < 0) {
		LOGE("[%s] write frame error!!\n", m_url.c_str());
		m_lastError = av_err2str(ret);
		return false;
	}

	return true;
}

void OutputStream::close()
{
	if (m_fmtCtx) {
		av_write_trailer(m_fmtCtx);
		// avio_close(m_fmtCtx->pb);
		avformat_free_context(m_fmtCtx);
		m_fmtCtx = NULL;
	}
}

#endif /* OUTPUTSTREAM_H_ */
