/*
 * ff.cpp
 *
 *  Created on: 2015年4月23日
 *      Author: zz
 */
#include "FFInputStream.h"
#include "FFOutputStream.h"
#include "TimestampReorder.h"

extern char *optarg;
extern int optind, opterr, optopt;
#include <getopt.h>


void usage(char* name) {
	printf("usage: %s -i inputFile [-f fmt] outputFile\n", name);
}



int main(int ac, char** av)
{
	//zlog_set_strlevel("ERROR");
	string inputFormat, inputUrl;
	string outputFormat, outputUrl;
	
	static struct option long_options[] = {
			{"help", 		0, 0, 'h'},
			{"input", 		1, 0, 'i'},
			{"outfmt", 		1, 0, 'f'},
			{"output", 		1, 0, 'c'},
			{0, 0, 0, 0}
	};
	
	int c;
	while ( (c = getopt_long(ac, av, "hi:f:", long_options, NULL)) != -1 ) {
		switch(c) {
		case 'h':
			usage(av[0]);exit(0);
			break;
		case 'i':
			inputUrl = optarg;
			break;
		case 'f':
			outputFormat = optarg;
			break;
		case '?':
			outputUrl = optarg;
			break;
		default:
			usage(av[0]);exit(0);
		}
	}
	
	outputUrl = av[ac - 1];
	
	LOGI("input: %s, fmt: %s, output: %s\n", inputUrl.c_str(), outputFormat.c_str(), outputUrl.c_str());
	


	av_register_all();
	avformat_network_init();



	InputStream inputStream;
	OutputStream outputStream;

	inputStream.setFormat(inputFormat);
	inputStream.setUrl(inputUrl);

	outputStream.setFormat(outputFormat);
	outputStream.setUrl(outputUrl);


	if(!inputStream.open()) {
		LOGE("open input(%s) failed: %s\n", inputUrl.c_str(), inputStream.getLastError().c_str());
		return 1;
	} 
	
	// outputStream.setStreamCodec(inputStream.getAudioStream(), inputStream.getVideoStream());
	outputStream.setiFmtCtx(inputStream.getFmtCtx());
	if(!outputStream.open()) {
		LOGE("open output(%s) failed: %s\n", outputUrl.c_str(), outputStream.getLastError().c_str());
		inputStream.close();
		return 2;
	} 

	

	TimestampReorder tsReorder;
	// TimestampReorder dtsReorder;
	// dtsReorder.tag = "dts";
	// ptsReorder.tag = "pts";
	while (true) {
		Packet pkt;
		bool ret = inputStream.readFrame(pkt);
		if (!ret) {
			if (inputStream.isEof()) {
				LOGI("File(%s) End!\n", inputFormat.c_str());
				break;
			} else {
				LOGE("ReadFile(%s) Error: %s\n", inputFormat.c_str(), inputStream.getLastError().c_str());
				continue;
				//goto error;
			}
		}
		
		DumpCpkt(pkt.c_pkt());
		int64_t pts, dts;
		pkt.getTimestamp(pts, dts);
		tsReorder.reorder(pts, dts, pkt.getStreamIndex() == inputStream.getVideoStreamIndex()? true: false);
		pkt.setTimestamp(pts, dts);
		DumpCpkt(pkt.c_pkt());
		//DumpCpkt2(pkt.c_pkt(), inputStream.getFmtCtx());
		ret = outputStream.writeFrame(pkt);
		
		if (!ret) {
			LOGE("write frame error!!\n", outputStream.getLastError().c_str());
			goto error;
		}
	}

	outputStream.close();
	inputStream.close();
	return 0;
error:
	LOGE("error. will exit!!\n");
	outputStream.close();
        inputStream.close();
	return -1;
}
