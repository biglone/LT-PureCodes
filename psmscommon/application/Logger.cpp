#if defined(_WIN32)
#else //_WIN32
#  include <stdlib.h>
#  include <string.h>
#endif //_WIN32

#include <stdio.h>
#include <iostream>
#include <new>

#include "Logger.h"
#include "cttk/Base.h"
#include "cttk/Thread.h"

namespace application
{

CLogger CLogger::m_instance;

CLogger& CLogger::GetInstance()
{
    return m_instance;
}

CLogger::CLogger()
: m_bInitialized(false)
, m_sPath("")
, m_sModule("")
, m_nPid(0)
, m_nLevel(LOGLEVEL_NONE)
, m_nCycle(LOGCYCLE_DAY)
, m_nCycleSeq(0)
, m_bPrint(false)
, m_bDebug(false)
{

}

CLogger::~CLogger()
{
    if (m_ofs.is_open()) m_ofs.close();
}

bool CLogger::Initialize(const std::string& rsPath, const std::string& rsModule, int nPid,
    int nLevel/* = LOGLEVEL_NONE*/, int nCycle/* = LOGCYCLE_DAY*/)
{
    do 
    {
        if (m_bInitialized) break;

        if (rsPath.length() <= 0) break;
        if (rsModule.length() <= 0) break;

        m_mutex.Lock();

        do 
        {
            m_sPath = rsPath;
            m_sModule = rsModule;
            m_nPid = nPid;
            SetLevel(nLevel);
            SetCycle(nCycle);

            if (!KeepFile()) break;

            m_bInitialized = true;
        } while (0);

        m_mutex.Unlock();
        
    } while (0);

    return m_bInitialized;
}

void CLogger::SetLevel(int nLevel)
{
    m_nLevel = nLevel;
}

void CLogger::SetCycle(int nCycle)
{
    m_nCycle = nCycle;
}

void CLogger::SetPrint(bool bPrint)
{
    m_bPrint = bPrint;
}

void CLogger::SetDebug(bool bDebug)
{
	m_bDebug = bDebug;
}

void CLogger::Log(int nLevel, const char* pszFun, const char* pszFmt, ...)
{
    m_mutex.Lock();

    do 
    {
		if (m_bDebug || (m_bPrint && (nLevel&m_nLevel) == nLevel)) 
		{
			printf("\n");

			printf("[%s][%s %d] ", 
				GetLevelStr(nLevel),
				cttk::datetime::currentdatetime().c_str(), 
				cttk::CThread::GetCurrentThreadId());

			va_list _arg ;
			va_start(_arg, pszFmt);
			vprintf(pszFmt, _arg);
			va_end(_arg) ;

			printf("\n");
		}

		if (!m_bInitialized) break;                 /// 没有初始化

		if ((nLevel&m_nLevel) != nLevel) break;     /// 不记录该等级日志
		if (!pszFun || strlen(pszFun) <= 0) break;  /// 参数不正确

        if (!KeepFile()) break;

        memset(m_szBuffer, 0, LOG_BUFFER_LEN);

        va_list _arg;
        va_start(_arg , pszFmt);
        cttk::str::cttk_vsnprintf(m_szBuffer, LOG_BUFFER_LEN, pszFmt, _arg);
        va_end(_arg);

        m_ofs << "[" << GetLevelStr(nLevel) << "]";
        m_ofs << cttk::datetime::currentdatetime() << " " << cttk::CThread::GetCurrentThreadId() << ":";
        m_ofs << pszFun << ":";
        m_ofs << m_szBuffer;
        m_ofs << std::endl;
    } while (0);

    m_mutex.Unlock();

    return;
}

bool CLogger::KeepFile()
{
    bool bRet = false;

    do 
    {
        int nCurrentCycleSeq = GetCurrentCycleSeq(m_nCycle);

        if (nCurrentCycleSeq != m_nCycleSeq)
        {
            if (m_ofs.is_open()) m_ofs.close();

            std::string sLogFile = cttk::str::assemble("%s%s%s_%d_%d.log",m_sPath.c_str(), FILE_SEPARATOR, m_sModule.c_str(), m_nPid, nCurrentCycleSeq);
            
			m_ofs.open(sLogFile.c_str(), std::ios::app);
            if (!m_ofs || !m_ofs.is_open()) break;

            m_ofs.rdbuf()->pubsetbuf(0, 0);

            m_nCycleSeq = nCurrentCycleSeq;
        }

        bRet = true;
    } while (0);
    
    return bRet;
}

int CLogger::GetCurrentCycleSeq(int nCycle)
{
    int nRet = 0;
    
    switch (nCycle)
    {
    case LOGCYCLE_HOUR  : nRet = (cttk::datetime::currenttime()+60*60*8)/(60*60); break;
    case LOGCYCLE_DAY   : nRet = (cttk::datetime::currenttime()+60*60*8)/(60*60*24); break;
    case LOGCYCLE_WEEK  : nRet = (cttk::datetime::currenttime()+60*60*8)/(60*60*24*7); break;
    default             : break;
    }

    return nRet;
}

const char* CLogger::GetLevelStr(int nLevel)
{
    const char* pszRet = "UNKNOW";

    switch (nLevel)
    {
    case LOGLEVEL_DEBUG : pszRet = "DEBUG"; break;
    case LOGLEVEL_INFO  : pszRet = "INFO" ; break;
    case LOGLEVEL_WARN  : pszRet = "WARN" ; break;
    case LOGLEVEL_ERROR : pszRet = "ERROR"; break;
    case LOGLEVEL_FATAL : pszRet = "FATAL"; break;
    default             : break;
    }

    return pszRet;
}

} // namespace cttk
