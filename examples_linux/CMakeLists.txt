
cmake_minimum_required(VERSION 3.0)

project(examples)

set(CMAKE_C_FLAGS   "-std=c11 -Wall -g")    # Initialized by the CFLAGS environment variable.
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -g")  # Initialized by the CXXFLAGS environment variable.


#set(PROJECT_ROOT_DIR /root/shareDir/poco-sample)
#include(${PROJECT_ROOT_DIR}/cmake/PocoMacros.cmake)
#include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
#link_directories(${CMAKE_CURRENT_LIST_DIR}/lib)

set(FFMPEG_LIBS libavdevice.so libavformat.so libavfilter.so libavcodec.so libswresample.so libswscale.so libavutil.so)
set(FFMPEG_LIBDS libavdeviced.so libavformatd.so libavfilterd.so libavcodecd.so libswresampled.so libswscaled.so libavutild.so)

set(SRCS_demuxing demuxing_example.c)
add_executable(demuxing_exe ${SRCS_demuxing})
target_link_libraries(demuxing_exe PUBLIC -lm ${FFMPEG_LIBDS})