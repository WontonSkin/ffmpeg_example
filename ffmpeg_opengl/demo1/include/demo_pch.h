#ifndef DEMO_PCH_H
#define DEMO_PCH_H

int demo_decode_video(const char *filename, const char *outfilename);
int demo_demuxing(const char *src_filename, const char *video_dst_filename, const char *audio_dst_filename);
int doRendering();

#endif
