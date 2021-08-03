#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"

using Poco::Logger;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::ConsoleChannel;
using Poco::FileChannel;
using Poco::Message;


#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <fstream>      // std::ofstream
#include "demo_log.h"

namespace DEMO
{

void initlog()
{  
    //FormattingChannel* pFCFile = new FormattingChannel(new PatternFormatter("%L%Y-%m-%d %H:%M:%S.%c [%I:%O:%u] %s:%q:%t"));
    FormattingChannel* pFCFile = new FormattingChannel(new PatternFormatter("%L%Y-%m-%d %H:%M:%S.%c %t"));
    pFCFile->setChannel(new FileChannel("demo1.log"));
    pFCFile->open();

    // create Logger objects.
    Logger& fileLogger = Logger::create("FileLogger", pFCFile, Message::PRIO_INFORMATION);

    // log some messages
    fileLogger.error("A error message");
    fileLogger.error("A warning message");
    fileLogger.information("A information message");
    poco_information(fileLogger, "Another informational message");
    poco_warning_f2(fileLogger, "A warning message with arguments: %d, %d", 1, 2);
    Logger::get("FileLogger").error("Another error message");
}


#define LINE_SZ 2048
#define TIME_SZ 32


std::ofstream ofs;

void openLogFile()
{
    ofs.open ("test.log", std::ofstream::out | std::ofstream::app);
}

void closeLogfile()
{
    ofs.close();
}


static unsigned long log_get_thread_id()
{
    //return pthread_self();
    return 0;
}

static const char *log_get_level_str(int level)
{
    switch (level) {
    case LOG_LEVEL_DEBUG:
        return "debug";
    case LOG_LEVEL_INFO:
        return "info";
    case LOG_LEVEL_WARN:
        return "warn";
    case LOG_LEVEL_ERROR:
        return "error";
    case LOG_LEVEL_FATAL:
        return "fatal";
    case LOG_LEVEL_TRACE:
        return "trace";
    default:
        return "unkown";
    }
}


static int log_tag_format(char *str,int size,const char*tag,int level)
{
    char time_buf[TIME_SZ] = {0};
    if(size<=0)
        return 0;
    //日志格式     级别、时间、当前线程号、tag
    //int ret=snprintf(str,size,"[%s][%s][0x%lx][%s] ",log_get_level_str(level),log_get_time (&time_buf[0],TIME_SZ),log_get_thread_id(),tag);
    //int ret=snprintf(str,size,"[%s][%s][%ld][%s] ",log_get_level_str(level),log_get_time (&time_buf[0],TIME_SZ),syscall(__NR_gettid),tag);
    int ret=snprintf(str,size,"[%s][%s] ",log_get_level_str(level),tag);
    if(ret>=size)
        ret=size-1;
    return ret;
}

static int log_msg_format(char *str,int size,const char *msg,va_list va)
{
    if(size<=0)
        return 0;
    int ret= vsnprintf(str,size,msg,va);
    if(ret>=size)
        ret=size-1;
    return ret;
}

static void log_output(const char* str)
{
    //std::cout << str << std::endl;    
    ofs << str;
}

static int log_vline(char *str, int size, const char* tag, int level, const char* msg, va_list va){
    char *pstr = str;
    int cnt=0;
    cnt=log_tag_format(pstr, size-2, tag, level);
    if(cnt<0) return 0;
    size-=cnt;
    pstr+=cnt;
    cnt=log_msg_format(pstr, size-2, msg, va);
    if(cnt<0) return 0;
    pstr+=cnt;
    if(*(pstr-1)!='\n'){
        *pstr='\n';
        pstr++;
        *pstr='\0';
    }
    log_output(str);
    return (pstr-str);
}

void log_line(const char* tag, int level, const char* format, ...)
{
    char buf[LINE_SZ] = {0};
    va_list va;
    va_start(va, format);
    (void)log_vline(&buf[0], LINE_SZ, tag, level, format, va);
    va_end(va);
}


}


