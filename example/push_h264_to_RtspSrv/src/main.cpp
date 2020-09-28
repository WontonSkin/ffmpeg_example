#include <string>
#include <memory>
#include <thread>
#include <iostream>

#include "pch.h"

using namespace std;

AVFormatContext *inputContext = nullptr;
AVFormatContext *outputContext = nullptr;

int OpenInput(string inputUrl)
{
	int ret = avformat_open_input(&inputContext, inputUrl.c_str(), nullptr, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Input file open input failed\n");
		return  ret;
	}

	ret = avformat_find_stream_info(inputContext, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Find input file stream inform failed\n");
		return ret;
	} else {
		av_log(NULL, AV_LOG_INFO, "Open input file  %s success\n",inputUrl.c_str());
	}

	return ret;
}

shared_ptr<AVPacket> ReadPacketFromSource()
{
	shared_ptr<AVPacket> packet(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket *p) { av_packet_free(&p); av_freep(&p);});
	av_init_packet(packet.get());
	int ret = av_read_frame(inputContext, packet.get());
	if (ret >= 0) {
		return packet;
	} else {
		return nullptr;
	}
}

void av_packet_rescale_ts(AVPacket *pkt, AVRational src_tb, AVRational dst_tb)
{
	if (pkt->pts != AV_NOPTS_VALUE)
		pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
	if (pkt->dts != AV_NOPTS_VALUE)
		pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
	if (pkt->duration > 0)
		pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
}


int OpenOutput(string outUrl)
{
	int ret = avformat_alloc_output_context2(&outputContext, nullptr, "rtsp", outUrl.c_str());
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "open output context failed\n");
		goto Error;
	}

	//ret = avio_open2(&outputContext->pb, outUrl.c_str(), AVIO_FLAG_READ_WRITE, nullptr, nullptr);	
	//if (ret < 0) {
	//	av_log(NULL, AV_LOG_ERROR, "open avio failed");
	//	goto Error;
	//}

	for (int i = 0; i < inputContext->nb_streams; i++)
	{
		AVStream * out_stream = avformat_new_stream(outputContext, NULL);

		ret = avcodec_parameters_copy(out_stream->codecpar, inputContext->streams[i]->codecpar);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "copy codec context failed");
			goto Error;
		}
		out_stream->codecpar->codec_tag = 0;
	}

	ret = avformat_write_header(outputContext, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "format write header failed");
		goto Error;
	}

	av_log(NULL, AV_LOG_FATAL, " Open output file success %s\n",outUrl.c_str());			
	return ret;

Error:
	if(outputContext) {
		avformat_close_input(&outputContext);
	}
	return ret;
}

void Init()
{
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
}

int main(int argc, char* argv[])
{
	Init();
	string input = "in.h264";
	string output = "rtsp://localhost/test";
	
	int ret = OpenInput(input);
	if (ret < 0) {
		exit(1);
	}
	std::cout << "[OpenInput over]===============================" << std::endl;
	
	ret = OpenOutput(output);
	if (ret < 0) {
		exit(1);
	}
	std::cout << "[OpenOutput over]===============================" << std::endl;

	int videoindex = 0;
	for (int i = 0; i < inputContext->nb_streams; i++) {
		if (inputContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	}

	int frame_index = 0;
	int64_t start_time = av_gettime();

	while(true)
	{
		shared_ptr<AVPacket> pkt = nullptr;
		
		pkt = ReadPacketFromSource();
		if (!pkt) {
			cout << "ReadPacketFromSource failed!" << endl;
			break;
		}

		if (pkt->pts == AV_NOPTS_VALUE) {

			AVRational time_base = inputContext->streams[videoindex]->time_base;
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(inputContext->streams[videoindex]->r_frame_rate);

			// Write PTS 
			pkt->pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base)*AV_TIME_BASE);
			pkt->dts = pkt->pts;
			pkt->duration = (double)calc_duration / (double)(av_q2d(time_base)*AV_TIME_BASE);
			cout << "pts:" << pkt->pts << " duration:" << pkt->duration << endl;

			// Delay 
			AVRational time_base_q = { 1,AV_TIME_BASE };
			int64_t pts_time = av_rescale_q(pkt->dts, time_base, time_base_q);
			int64_t now_time = av_gettime() - start_time;
			if (pts_time > now_time)
				av_usleep(pts_time - now_time);
		}

		// convert PTS
		AVStream *in_stream = inputContext->streams[pkt->stream_index];
		AVStream *out_stream = outputContext->streams[pkt->stream_index];
		av_packet_rescale_ts(pkt.get(), in_stream->time_base, out_stream->time_base);
		cout << "pkt->pts:" << pkt->pts << " pkt->duration:" << pkt->duration <<endl;

		//write packet
		ret =  av_interleaved_write_frame(outputContext, pkt.get());
		if (ret >= 0 ) {
			cout << "WritePacket Success, frame_index:"  << frame_index  << endl;
		} else if (ret < 0){
			cout << "WritePacket failed! ret = " << ret << endl;
			break;
		}

		frame_index++;
	}

	avformat_free_context(outputContext);
	avformat_close_input(&inputContext);

	return 0;
}

