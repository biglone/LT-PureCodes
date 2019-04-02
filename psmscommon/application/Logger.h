///@file Logger.h 日志类头文件.

#ifndef _LOGGER_H_INCLUDED
#define _LOGGER_H_INCLUDED

#include <string>
#include <fstream>
#include "cttk/Mutex.h"

namespace application
{

// 日志等级
#define LOGLEVEL_DEBUG  0x01    ///< 调试信息
#define LOGLEVEL_INFO   0x02    ///< 提示信息
#define LOGLEVEL_WARN   0x04    ///< 告警信息
#define LOGLEVEL_ERROR  0x08    ///< 错误信息
#define LOGLEVEL_FATAL  0x10    ///< 致命信息

#define LOGLEVEL_NONE   0x00    ///< 无等级
#define LOGLEVEL_ALL    LOGLEVEL_DEBUG|LOGLEVEL_INFO|LOGLEVEL_WARN|LOGLEVEL_ERROR|LOGLEVEL_FATAL ///< 全部等级

// 日志周期
#define LOGCYCLE_HOUR   0x01    ///< 每小时
#define LOGCYCLE_DAY    0x02    ///< 每天
#define LOGCYCLE_WEEK   0x04    ///< 每星期

// 日志操作
#define LOGGER_INIT(path,module,pid) CLogger::GetInstance().Initialize(path,module,pid) ///< 日志初始化
#define LOGGER_SETLEVEL(v) CLogger::GetInstance().SetLevel(v)							///< 设置日志等级
#define LOGGER_SETCYCLE(v) CLogger::GetInstance().SetCycle(v)							///< 设置日志周期
#define LOGGER_SETPRINT(v) CLogger::GetInstance().SetPrint(v)							///< 设置打印开关
#define LOGGER_SETDEBUG(v) CLogger::GetInstance().SetDebug(v)							///< 设置调试开关

#define LOG_DEBUG(fmt,...) CLogger::GetInstance().Log(LOGLEVEL_DEBUG, __FUNCTION__, fmt, ##__VA_ARGS__)   ///< 记录调试信息
#define LOG_INFO(fmt,...)  CLogger::GetInstance().Log(LOGLEVEL_INFO , __FUNCTION__, fmt, ##__VA_ARGS__)   ///< 记录提示信息
#define LOG_WARN(fmt,...)  CLogger::GetInstance().Log(LOGLEVEL_WARN , __FUNCTION__, fmt, ##__VA_ARGS__)   ///< 记录告警信息
#define LOG_ERROR(fmt,...) CLogger::GetInstance().Log(LOGLEVEL_ERROR, __FUNCTION__, fmt, ##__VA_ARGS__)   ///< 记录错误信息
#define LOG_FATAl(fmt,...) CLogger::GetInstance().Log(LOGLEVEL_FATAL, __FUNCTION__, fmt, ##__VA_ARGS__)   ///< 记录致命信息

/**
* @brief 日志类.
* @version 1.0
* @date 2010-11-23
*/
class CLogger
{
    /// 单条日志信息最大长度
    static const int LOG_BUFFER_LEN = 1024*10;

private:
    CLogger();
    ~CLogger();
    CLogger(const CLogger& rLogger);
    CLogger& operator=( const CLogger& rLogger);

public:
    /// 获取实例
    static CLogger& GetInstance();

public:
    /// 初始化
    bool Initialize(const std::string& rsPath, const std::string& rsModule, int nPid,
        int nLevel = LOGLEVEL_NONE, int nCycle = LOGCYCLE_DAY);

    /// 设置日志等级
    void SetLevel(int nLevel);

    /// 设置日志周期
    void SetCycle(int nCycle);

    /// 设置打印开关
    void SetPrint(bool bPrint = true);

	/// 设置调试开关
	void SetDebug(bool bDebug = true);

    /// 写日志
    void Log(int nLevel, const char* pszFun, const char* pszFmt, ...);

private:
    bool KeepFile();
    int GetCurrentCycleSeq(int nCycle);
    const char* GetLevelStr(int nLevel);

private:
    static CLogger m_instance;                  // 实例
    bool m_bInitialized;                        // 是否已经初始化
    std::string m_sPath;                        // 日志存放路径
    std::string m_sModule;                      // 应用程序模块名
    int m_nPid;                                 // 进程号
    int m_nLevel;                               // 日志等级
    int m_nCycle;                               // 日志周期
    int m_nCycleSeq;                            // 日志周期序列号
    std::ofstream m_ofs;                        // 输出流
    bool m_bPrint;                              // 是否打印到屏幕
	bool m_bDebug;                              // 是否调试
    char m_szBuffer[LOG_BUFFER_LEN];            // 缓存
    cttk::CMutex m_mutex;                       // 线程安全锁
};

} // namespace application

#endif //_LOGGER_H_INCLUDED
