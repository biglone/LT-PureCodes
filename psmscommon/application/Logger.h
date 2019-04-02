///@file Logger.h ��־��ͷ�ļ�.

#ifndef _LOGGER_H_INCLUDED
#define _LOGGER_H_INCLUDED

#include <string>
#include <fstream>
#include "cttk/Mutex.h"

namespace application
{

// ��־�ȼ�
#define LOGLEVEL_DEBUG  0x01    ///< ������Ϣ
#define LOGLEVEL_INFO   0x02    ///< ��ʾ��Ϣ
#define LOGLEVEL_WARN   0x04    ///< �澯��Ϣ
#define LOGLEVEL_ERROR  0x08    ///< ������Ϣ
#define LOGLEVEL_FATAL  0x10    ///< ������Ϣ

#define LOGLEVEL_NONE   0x00    ///< �޵ȼ�
#define LOGLEVEL_ALL    LOGLEVEL_DEBUG|LOGLEVEL_INFO|LOGLEVEL_WARN|LOGLEVEL_ERROR|LOGLEVEL_FATAL ///< ȫ���ȼ�

// ��־����
#define LOGCYCLE_HOUR   0x01    ///< ÿСʱ
#define LOGCYCLE_DAY    0x02    ///< ÿ��
#define LOGCYCLE_WEEK   0x04    ///< ÿ����

// ��־����
#define LOGGER_INIT(path,module,pid) CLogger::GetInstance().Initialize(path,module,pid) ///< ��־��ʼ��
#define LOGGER_SETLEVEL(v) CLogger::GetInstance().SetLevel(v)							///< ������־�ȼ�
#define LOGGER_SETCYCLE(v) CLogger::GetInstance().SetCycle(v)							///< ������־����
#define LOGGER_SETPRINT(v) CLogger::GetInstance().SetPrint(v)							///< ���ô�ӡ����
#define LOGGER_SETDEBUG(v) CLogger::GetInstance().SetDebug(v)							///< ���õ��Կ���

#define LOG_DEBUG(fmt,...) CLogger::GetInstance().Log(LOGLEVEL_DEBUG, __FUNCTION__, fmt, ##__VA_ARGS__)   ///< ��¼������Ϣ
#define LOG_INFO(fmt,...)  CLogger::GetInstance().Log(LOGLEVEL_INFO , __FUNCTION__, fmt, ##__VA_ARGS__)   ///< ��¼��ʾ��Ϣ
#define LOG_WARN(fmt,...)  CLogger::GetInstance().Log(LOGLEVEL_WARN , __FUNCTION__, fmt, ##__VA_ARGS__)   ///< ��¼�澯��Ϣ
#define LOG_ERROR(fmt,...) CLogger::GetInstance().Log(LOGLEVEL_ERROR, __FUNCTION__, fmt, ##__VA_ARGS__)   ///< ��¼������Ϣ
#define LOG_FATAl(fmt,...) CLogger::GetInstance().Log(LOGLEVEL_FATAL, __FUNCTION__, fmt, ##__VA_ARGS__)   ///< ��¼������Ϣ

/**
* @brief ��־��.
* @version 1.0
* @date 2010-11-23
*/
class CLogger
{
    /// ������־��Ϣ��󳤶�
    static const int LOG_BUFFER_LEN = 1024*10;

private:
    CLogger();
    ~CLogger();
    CLogger(const CLogger& rLogger);
    CLogger& operator=( const CLogger& rLogger);

public:
    /// ��ȡʵ��
    static CLogger& GetInstance();

public:
    /// ��ʼ��
    bool Initialize(const std::string& rsPath, const std::string& rsModule, int nPid,
        int nLevel = LOGLEVEL_NONE, int nCycle = LOGCYCLE_DAY);

    /// ������־�ȼ�
    void SetLevel(int nLevel);

    /// ������־����
    void SetCycle(int nCycle);

    /// ���ô�ӡ����
    void SetPrint(bool bPrint = true);

	/// ���õ��Կ���
	void SetDebug(bool bDebug = true);

    /// д��־
    void Log(int nLevel, const char* pszFun, const char* pszFmt, ...);

private:
    bool KeepFile();
    int GetCurrentCycleSeq(int nCycle);
    const char* GetLevelStr(int nLevel);

private:
    static CLogger m_instance;                  // ʵ��
    bool m_bInitialized;                        // �Ƿ��Ѿ���ʼ��
    std::string m_sPath;                        // ��־���·��
    std::string m_sModule;                      // Ӧ�ó���ģ����
    int m_nPid;                                 // ���̺�
    int m_nLevel;                               // ��־�ȼ�
    int m_nCycle;                               // ��־����
    int m_nCycleSeq;                            // ��־�������к�
    std::ofstream m_ofs;                        // �����
    bool m_bPrint;                              // �Ƿ��ӡ����Ļ
	bool m_bDebug;                              // �Ƿ����
    char m_szBuffer[LOG_BUFFER_LEN];            // ����
    cttk::CMutex m_mutex;                       // �̰߳�ȫ��
};

} // namespace application

#endif //_LOGGER_H_INCLUDED
