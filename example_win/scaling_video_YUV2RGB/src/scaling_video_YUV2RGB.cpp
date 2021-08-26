/*
 * Copyright (c) 2012 Stefano Sabatini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>

#include "pch.h"

using namespace std;

#define SUCCESS           0
#define ERR_COMMON_FAIL (-1)
#define ERR_END_OF_FILE (-2)

/*
	A graphical illustration: Each letter represents one bit.
	->For a single I420 pixel: YYYYYYYY UU VV
	->For a 50-pixel I420 frame: YYYYYYYY×50 UU×50 VV×50 (or Y×8×50 U×2×50 V×2×50 for short)
	->For an n-pixel I420 frame: Y×8×n U×2×n V×2×n
	*/
static int read_yuv420p_image(uint8_t *data[4], int width, int height, FILE *pFile)
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


// src_yuv420p_320x240.yuv - > dst_rgb24_320x240.rgb
int main(int argc, char **argv)
{
	uint8_t *src_data[4], *dst_data[4];
	int src_linesize[4], dst_linesize[4];
	int dst_bufsize;
	int ret;
	struct SwsContext *sws_ctx = NULL;

	/*
	1、手动输入源文件属性：
	分辨率，例如：src_w = 320, src_h = 240
	像素格式，例如：AV_PIX_FMT_YUV420P、AV_PIX_FMT_RGB24
	*/
	string src_filename("src_yuv420p_320x240.yuv");
	string src_size("qvga");          // { "qvga",      320, 240 }

	int src_w = 0; 
	int src_h = 0; 
	if (av_parse_video_size(&src_w, &src_h, src_size.c_str()) < 0) {
		fprintf(stderr,
			"Invalid size '%s', must be in the form WxH or a valid size abbreviation\n", src_size.c_str());
		exit(1);
	}

	enum AVPixelFormat src_pix_fmt = AV_PIX_FMT_YUV420P;

	/*
	2、手动输入目标文件属性：
	分辨率，例如：dst_w = 320, dst_h = 240
	像素格式，例如：AV_PIX_FMT_YUV420P、AV_PIX_FMT_RGB24
	*/
	string dst_filename("dst_rgb24_320x240.rgb");
	string dst_size("qvga");          // { "qvga",      320, 240 },

	int dst_w = 0;
	int dst_h = 0;
	if (av_parse_video_size(&dst_w, &dst_h, dst_size.c_str()) < 0) {
		fprintf(stderr,
			"Invalid size '%s', must be in the form WxH or a valid size abbreviation\n", dst_size.c_str());
		exit(1);
	}

	enum AVPixelFormat dst_pix_fmt = AV_PIX_FMT_RGB24;

	/*
	3、准备工作
		3.1 打开源文件
		3.2 打开目标文件
		3.3 申请源帧内存
		3.4 申请目标帧内存
	*/
	FILE *src_file = NULL;
	src_file = fopen(src_filename.c_str(), "rb");
	if (!src_file) {
		fprintf(stderr, "Could not open destination file %s\n", src_filename.c_str());
		exit(1);
	}

	FILE *dst_file = NULL;
	dst_file = fopen(dst_filename.c_str(), "wb");
	if (!dst_file) {
		fprintf(stderr, "Could not open destination file %s\n", dst_filename.c_str());
		goto end;
	}

	/* create scaling context */
	sws_ctx = sws_getContext(src_w, src_h, src_pix_fmt, dst_w, dst_h, dst_pix_fmt,
		SWS_BILINEAR, NULL, NULL, NULL);
	if (!sws_ctx) {
		fprintf(stderr, "Impossible to create scale context for the conversion fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
			av_get_pix_fmt_name(src_pix_fmt), src_w, src_h, av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h);
		ret = AVERROR(EINVAL);
		goto end;
	}

	/* allocate source and destination image buffers */
	if ((ret = av_image_alloc(src_data, src_linesize, src_w, src_h, src_pix_fmt, 16)) < 0) {
		fprintf(stderr, "Could not allocate source image\n");
		goto end;
	}
	cout << "################" << endl;
	cout << "src_linesize[0]:" << src_linesize[0] << endl;
	cout << "src_linesize[1]:" << src_linesize[1] << endl;
	cout << "src_linesize[2]:" << src_linesize[2] << endl;
	cout << "src_linesize[3]:" << src_linesize[3] << endl;

	/* buffer is going to be written to rawvideo file, no alignment */
	if ((ret = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pix_fmt, 1)) < 0) {
		fprintf(stderr, "Could not allocate destination image\n");
		goto end;
	}
	dst_bufsize = ret;
	cout << "################" << endl;
	cout << "dst_linesize[0]:" << dst_linesize[0] << endl;
	cout << "dst_linesize[1]:" << dst_linesize[1] << endl;
	cout << "dst_linesize[2]:" << dst_linesize[2] << endl;
	cout << "dst_linesize[3]:" << dst_linesize[3] << endl;

	/*
	4、循环转换
	while {
		4.1 读入一行源文件
		4.2 转换
		4.3 将转换结果写入目标文件
	}
	*/

	// src_yuv420p_320x240.yuv - > dst_rgb24_320x240.rgb
	while (1) {
		/* generate synthetic video */
		ret = read_yuv420p_image(src_data, src_w, src_h, src_file);
		if (ret != SUCCESS) {
			if (ret == ERR_END_OF_FILE) {
				break;
			}
			else {
				fprintf(stderr, "read_yuv420p_image fail.\n");
				break;
			}
		}

		/* convert to destination format */
		sws_scale(sws_ctx, (const uint8_t * const*)src_data,
			src_linesize, 0, src_h, dst_data, dst_linesize);

		/* write scaled image to file */
		fwrite(dst_data[0], 1, dst_bufsize, dst_file);
	}

	fprintf(stderr, "Scaling succeeded. Play the output file with the command:\n"
		"ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
		av_get_pix_fmt_name(dst_pix_fmt), dst_w, dst_h, dst_filename.c_str());

end:
	if (src_file != NULL) fclose(src_file);
	if (dst_file != NULL) fclose(dst_file);
	av_freep(&src_data[0]);
	av_freep(&dst_data[0]);
	sws_freeContext(sws_ctx);
	return ret < 0;
}

