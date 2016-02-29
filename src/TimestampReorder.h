
/*
	将潜在的不连续的时间戳，调整的单增且连续。
 */


#ifndef TIMESTAMPREORDER_H
#define TIMESTAMPREORDER_H

#include <stdio.h>
#include <assert.h>

#define TS_MAX(a, b) (a)>(b)? (a):(b)

#define UNINIT_TIMESTAMP (~(int64_t)0)
class TimestampReorder
{
	enum { apts, adts, vpts, vdts, tsnum };
public:
	string tag;
	TimestampReorder();

	void reorder(int64_t& pts, int64_t& dts, bool isVideo);
private:
	int64_t last_iTs[tsnum];
	int64_t last_oTs[tsnum];

	void reorderVideo(int64_t& pts, int64_t& dts);
	void reorderAudio(int64_t& pts, int64_t& dts);
};

TimestampReorder::TimestampReorder()
{
	for (int i=0; i<tsnum; i++) {
		last_iTs[i] = last_oTs[i] = UNINIT_TIMESTAMP;
	}
}

void TimestampReorder::reorderVideo(int64_t& pts, int64_t& dts)
{
	int64_t ptsOffset, dtsOffset;

	if (last_iTs[adts] == UNINIT_TIMESTAMP) {
		// 以 ~！~视频~！~ 时间戳为基准，进行调整
		if (last_iTs[vdts] == UNINIT_TIMESTAMP) {
			ptsOffset = dtsOffset = 0;
			last_oTs[vpts] = 0;
			LOGI("dts(%lli) - pts(%lli) = %lli\n", dts, pts, dts - pts);
			last_oTs[vdts] = dts - pts;
		} else {
			ptsOffset = pts - last_iTs[vpts];
			dtsOffset = dts - last_iTs[vdts];
			if (dtsOffset<=0 || dtsOffset > 1000) {  // 时间戳异常，修正
				LOGI("video timestamp exception!\n");
				ptsOffset = dtsOffset = 1;
			}
		}

		last_oTs[vpts] += ptsOffset;
		last_oTs[vdts] += dtsOffset;
		last_iTs[vpts] = pts;
		last_iTs[vdts] = dts;
	} else {  
		// 以 ~！~音频~！~ 时间戳为基准，进行调整
		ptsOffset = pts - last_iTs[apts];
		dtsOffset = dts - last_iTs[adts];

		if (llabs(dtsOffset)>800) {	// 时间戳异常进行修正
			LOGI("video timestamp find exception!\n");
			ptsOffset = dtsOffset = 1;
		}

		int64_t tmp = dtsOffset + last_oTs[adts];
		if (tmp <= last_oTs[vdts]) {
			// something error
			last_oTs[vpts] += 1;
			last_oTs[vdts] += 1;
		} else {
			last_oTs[vpts] = ptsOffset + last_oTs[apts];
			last_oTs[vdts] = dtsOffset + last_oTs[adts];
		}
		// LOGI("last_oTs[vpts]:%lli, last_oTs[vdts]: %lli\n", last_oTs[vpts], last_oTs[vdts]);
	}
	
	pts = last_oTs[vpts];
	dts = last_oTs[vdts];
}

void TimestampReorder::reorderAudio(int64_t& pts, int64_t& dts)
{
	int64_t ptsOffset, dtsOffset;

	if (last_iTs[adts] == UNINIT_TIMESTAMP) { // 初始化各种成员变量

		if (last_iTs[vdts] == UNINIT_TIMESTAMP) {
			ptsOffset = dtsOffset = 0;
			last_oTs[apts] = 0;
			last_oTs[adts] = 0;
		} else {
			// ptsOffset (MUST)>= dtsOffset
			ptsOffset = pts - last_iTs[vdts];
			dtsOffset = dts - last_iTs[vdts];
			last_oTs[apts] = last_oTs[vpts];
			last_oTs[adts] = last_oTs[vdts];
		}

		if (llabs(ptsOffset) > 500) {  // 初始值存在异常，进行修正
			LOGI("init first auido timestamp exception!\n");
			ptsOffset = dtsOffset = 1;
		}

	} else { // 音频时间戳已自身为基准，进行调整
		ptsOffset = pts - last_iTs[apts];
		dtsOffset = dts - last_iTs[adts];

		if (dtsOffset<=0 || dtsOffset>800) {	// 时间戳异常进行修正
			LOGI("auido timestamp find exception!\n");
			ptsOffset = dtsOffset = 1;
		}
	}

	last_oTs[apts] += ptsOffset;
	last_oTs[adts] += dtsOffset;

	last_iTs[apts] = pts;
	last_iTs[adts] = dts;

	pts = last_oTs[apts];
	dts = last_oTs[adts];
}


void TimestampReorder::reorder(int64_t& pts, int64_t& dts, bool isVideo)
{
	if (isVideo) {
		reorderVideo(pts, dts);
	} else {
		reorderAudio(pts, dts);
	}
}

#endif
