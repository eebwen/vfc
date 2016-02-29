#ifndef FFINPUTSTREAM_H
#define FFINPUTSTREAM_H

#include "FFHeader.h"

#include <map>

using std::map;

class InputStream{
	enum {videoStreamIndex = 0, audioStreamIndex, avStreamNum};
public:
	InputStream();
	virtual ~InputStream();

	bool open();
	void close();
	
	void setUrl(string& url) { m_url = url; }
	void setFormat(string& format) { m_format = format; }
	
	bool readFrame(Packet& pkt);
	void rescaleTimestamp(Packet& pkt) {
		AVRational rational = {1, 1000};
		AVPacket* c_pkt = pkt.c_pkt();
		c_pkt->pts = av_rescale_q(c_pkt->pts, m_avstream[c_pkt->stream_index]->time_base, rational);
		c_pkt->dts = av_rescale_q(c_pkt->dts, m_avstream[c_pkt->stream_index]->time_base, rational);
	}

	// 属性设置
	void setUrl(const string& url);
	void addOption(const string& key, const string& value);
	
	string getLastError() { return m_lastError; }
	int isEof() { return m_isEof; }
	
	AVStream* getAudioStream() {return m_avstream[audioStreamIndex];}
	AVStream* getVideoStream() {return m_avstream[videoStreamIndex];}
	
	AVFormatContext* getFmtCtx() {return m_fmtCtx;}
	
	int getVideoStreamIndex() { return m_avstreamIndex[videoStreamIndex]; }
private:
	string m_url; // 输入地址
	string m_format; // 文件格式
	string m_lastError;
	
	typedef map<string, string> Options;
	Options m_options; 	// 打开摄像头时设置的选项, 调用者需确保option可用
	
	int 	m_isEof;
	int 		m_avstreamIndex[avStreamNum];
	AVStream* 	m_avstream[avStreamNum];
	
	AVFormatContext* m_fmtCtx;
};

InputStream::InputStream():m_isEof(0), m_avstreamIndex({0}), m_avstream({0}){m_fmtCtx = NULL;}

InputStream::~InputStream() {close();}


bool InputStream::open() {
	AVDictionary* options = NULL;
	for(Options::iterator it = m_options.begin(); it != m_options.end() && !m_options.empty(); ++it) {
		const char* key = it->first.c_str();
		const char* value = it->second.c_str();
		av_dict_set(&options, key, value, 0);
	}

	if (m_url.empty()) {
		m_lastError = "null url";
		return false;
	}

	if (avformat_open_input(&m_fmtCtx, m_url.c_str(), NULL, &options)<0){
		 LOGE("Cannot open input stream: %s\n", m_url.c_str());
		 return false;
	}

	if (avformat_find_stream_info(m_fmtCtx, NULL)) {
		LOGE("Cannot find stream information for input %s\n", m_url.c_str());
		return false;
	}

	for (unsigned int i=0; i<m_fmtCtx->nb_streams; i++) {
        AVStream *stream = m_fmtCtx->streams[i];
        AVCodecContext *ctx = stream->codec;
        if (ctx->codec_type == AVMEDIA_TYPE_VIDEO || ctx->codec_type == AVMEDIA_TYPE_AUDIO) {

        	if (ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        		m_avstreamIndex[audioStreamIndex] = i;
				m_avstream[audioStreamIndex] = stream;
        	} else if (ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
				m_avstreamIndex[videoStreamIndex] = i;
				m_avstream[videoStreamIndex] = stream;
        	}

        	
        	// int ret = avcodec_open2(ctx, avcodec_find_decoder(ctx->codec_id), NULL);
        	// if (ret < 0) {
        		// LOGE("Failed to open decoder for stream #%u for input %s\n", i, m_url.c_str());
        		// return false;
        	// }
        }
	}

	av_dump_format(m_fmtCtx, 0, m_url.c_str(), 0);
	return true;
}


bool InputStream::readFrame(Packet& pkt) {
	int ret = av_read_frame(m_fmtCtx, pkt.c_pkt());
	if (ret == 0) {
		rescaleTimestamp(pkt);
		if (pkt.getStreamIndex() == m_avstreamIndex[audioStreamIndex]) {
			pkt.setStreamIndex(1);
		} else {
			pkt.setStreamIndex(0);
		}
		
		return true;
	} else {
		if (ret == AVERROR_EOF) m_isEof = 1;
		m_lastError = av_err2str(ret);
		return false;
	}
}

void InputStream::close() {
	if(m_fmtCtx) {
		// if (m_audioStream->codec) avcodec_close(m_audioStream->codec);
		// if (m_videoStream->codec) avcodec_close(m_videoStream->codec);
		avformat_close_input(&m_fmtCtx);
		m_fmtCtx = NULL;
	}
}

#endif