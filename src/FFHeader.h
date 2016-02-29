#ifndef HEADER_H_
#define HEADER_H_

#define INT64_C
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include "zlog.h"
#include <string>
using std::string;


#define DumpCpkt(cpkt) \
	LOGI("index = %d, pts = %lli, dts = %lli\n", cpkt->stream_index, cpkt->pts, cpkt->dts);
	
#define DumpCpkt2(cpkt, fmtCtx) \
	av_pkt_dump2(NULL, cpkt, 0, fmtCtx->streams[cpkt->stream_index]);

class Packet
{
public:
	Packet() {
		av_init_packet(&m_pkt);
		m_pkt.data = NULL;
		m_pkt.size = 0;
	}
	~Packet() {
		av_free_packet(&m_pkt);
	}

	AVPacket* c_pkt(void) {
		return &m_pkt;
	}

	void setStreamIndex(int index) {
		m_pkt.stream_index = index;
	}

	int getStreamIndex(void) {
		return m_pkt.stream_index;
	}

	void setTimestamp(int64_t pts, int64_t dts) {
		m_pkt.pts = pts;
		m_pkt.dts = dts;
	}

	void getTimestamp(int64_t& pts, int64_t& dts) {
		pts = m_pkt.pts;
		dts = m_pkt.dts;
	}

private:
	AVPacket m_pkt;
};

#endif