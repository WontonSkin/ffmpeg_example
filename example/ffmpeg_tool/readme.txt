
附加包含目录：
其路径为ffmpeg4.2.1源码路径(编译之后的)

[1]
调试参数：
将本地h264裸数据通过rtsp推流到服务器
-re -i input.h264 -rtsp_transport tcp -vcodec h264 -f rtsp rtsp://localhost/test

[2]
调试参数：
将本地1080P的h264裸数据进行缩放
-i 1920x1080.h264 -s 352x288 352x288.h264

[3]
调试参数：
对h264解码为YUV数据
-i 352x288.h264 -vcodec rawvideo out.yuv
-i 352x288.h264 -vcodec rawvideo -an out.yuv
其中 -an 表示不处理音频
其中 -vn 表示不处理视频

[4]
调试参数：
对h264解码为YUV数据
-i 352x288.h264 out.rtp



