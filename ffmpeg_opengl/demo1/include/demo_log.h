#ifndef __DEMO_LOG_HH__
#define __DEMO_LOG_HH__

namespace DEMO
{

//void initlog();

void log_line(const char* tag, int level, const char* format, ...);

void openLogFile();

void closeLogfile();

#define LOG_LEVEL_FATAL     8
#define LOG_LEVEL_ERROR     16
#define LOG_LEVEL_WARN      24
#define LOG_LEVEL_INFO      32
#define LOG_LEVEL_DEBUG     48
#define LOG_LEVEL_TRACE     56
#define LOG_LEVEL_DEFINE    LOG_LEVEL_INFO

#define DemoLog(level, format, ...) do {if (level <= LOG_LEVEL_DEFINE) log_line("DEMO", level, "%s:%d " format, __FUNCTION__, __LINE__, ##__VA_ARGS__);} while (0)

}


#endif
