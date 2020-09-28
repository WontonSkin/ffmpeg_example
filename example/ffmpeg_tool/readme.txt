
附加包含目录：
其路径为ffmpeg4.2.1源码路径(编译之后的)

调试参数：
将本地h264裸数据通过rtsp推流到服务器
-re -i input.h264 -rtsp_transport tcp -vcodec h264 -f rtsp rtsp://localhost/test
