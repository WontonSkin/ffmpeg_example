
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pch.h"

#define SUCCESS           0
#define ERR_COMMON_FAIL (-1)
#define ERR_END_OF_FILE (-2)


/*
	A graphical illustration: Each letter represents one bit.
	->For a single I420 pixel: YYYYYYYY UU VV
	->For a 50-pixel I420 frame: YYYYYYYY×50 UU×50 VV×50 (or Y×8×50 U×2×50 V×2×50 for short)
	->For an n-pixel I420 frame: Y×8×n U×2×n V×2×n
	*/
static int read_yuv420p_image1(uint8_t *data[4], int width, int height, FILE *pFile)
{
	int ret = 0;
	int Ylen = width * height;
	int Ulen = width * height / 4;
	int Vlen = width * height / 4;

	ret = fread(data[0], 1, Ylen, pFile);
	if (ret != Ylen) {
		if (0 == ret) {
			fprintf(stdout, "read end of file.\n");
			return ERR_END_OF_FILE;
		}
		else {
			fprintf(stderr, "fread Ylen fail:%d.\n", ret);
			return ERR_COMMON_FAIL;
		}
	}

	ret = fread(data[1], 1, Ulen, pFile);
	if (ret != Ulen) {
		fprintf(stderr, "fread Ulen fail:%d.\n", ret);
		return ERR_COMMON_FAIL;
	}

	ret = fread(data[2], 1, Vlen, pFile);
	if (ret != Vlen) {
		fprintf(stderr, "fread Vlen fail:%d.\n", ret);
		return ERR_COMMON_FAIL;
	}

	return SUCCESS;
}

static void encode1(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
	FILE *outfile)
{
	int ret;

	/* send the frame to the encoder */
	if (frame)
		printf("Send frame %3ld\n", frame->pts);

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		printf("Write packet %3ld (size=%5d)\n", pkt->pts, pkt->size);
		fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt);
	}
}

int main(int argc, char **argv)
{
	const char *infilename, *outfilename;
	const AVCodec *codec;
	AVCodecContext *c = NULL;
	int ret = 0;
	AVFrame *frame;
	AVPacket *pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <output file> <codec name>\n", argv[0]);
		exit(0);
	}
	infilename = argv[1];
	outfilename = argv[2];

	/* find the libx264 encoder */
	codec = avcodec_find_encoder_by_name("libx264");
	if (!codec) {
		fprintf(stderr, "Codec libx264 not found\n");
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	/* put sample parameters */
	c->bit_rate = 4000000;
	/* resolution must be a multiple of two */
	c->width = 1920;
	c->height = 1080;
	/* frames per second */
	c->time_base = { 1, 25 }; // (AVRational) { 1, 25 };
	c->framerate = { 25, 1 }; // (AVRational) { 25, 1 };

	/* emit one intra frame every ten frames
	 * check frame pict_type before passing frame
	 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	 * then gop_size is ignored and the output of encoder
	 * will always be I frame irrespective to gop_size
	 */
	c->gop_size = 25;
	c->max_b_frames = 0;               //无B帧设置
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	/* open it */
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
		exit(1);
	}

	//源文件处理
	FILE *src_file = NULL;
	src_file = fopen(infilename, "rb");
	if (!src_file) {
		fprintf(stderr, "Could not open destination file %s\n", infilename);
		exit(1);
	}
	
	//目标文件处理
	FILE *dst_file = fopen(outfilename, "wb");
	if (!dst_file) {
		fprintf(stderr, "Could not open %s\n", outfilename);
		exit(1);
	}

	//AVPacket处理
	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	//AVFrame处理
	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;

	ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}

	/* encode 1 second of video */
	int i = 0;
	while (1) {
		/* make sure the frame data is writable */
		ret = av_frame_make_writable(frame);
		if (ret < 0)
			exit(1);

		/* get YUV frame */
		ret = read_yuv420p_image1(frame->data, frame->width, frame->height, src_file);
		if (ret != SUCCESS) {
			if (ret == ERR_END_OF_FILE) {
				break;
			}
			else {
				fprintf(stderr, "read_yuv420p_image fail.\n");
				break;
			}
		}

		frame->pts = i++;

		/* encode the image */
		encode1(c, frame, pkt, dst_file);
	}

	/* flush the encoder */
	encode1(c, NULL, pkt, dst_file);

	/* add sequence end code to have a real MPEG file */
	fwrite(endcode, 1, sizeof(endcode), dst_file);
	fclose(dst_file);
	fclose(src_file);

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	return 0;
}
