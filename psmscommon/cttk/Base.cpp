#if defined(_WIN32)
#  include <stdio.h>
#  include <io.h>
#else //_WIN32
#  include <stdlib.h>
#  include <string.h>
#  include <dirent.h>
#  include <fnmatch.h>
#  include <sys/stat.h>
#endif //_WIN32

#include <time.h>
#include <fstream>
#include <sstream>

#include "Base.h"
#include "Thread.h"

namespace cttk
{

//==============================================================================

namespace dbg
{
    static bool s_cttk_dbg_show = true;

    void show(bool b/* = true*/)
    {
        s_cttk_dbg_show = b;
    }

    void print(const char* fmt, ...)
    {
        if (s_cttk_dbg_show)
        {
            printf("\n");

            printf("[%s %d] ", cttk::datetime::currentdatetime().c_str(), CThread::GetCurrentThreadId());

	        va_list _arg ;
	        va_start(_arg, fmt);
	        vprintf(fmt, _arg);
	        va_end(_arg) ;

            printf("\n");
        }
    }

} // namespace dbg

//==============================================================================

namespace sys
{
    void sleep(int s)
    {
        CThread::sleep(s);
    }

    void usleep(int ms)
    {
        CThread::usleep(ms);
    }
} // sys

//==============================================================================

namespace datetime
{
    static const int STRLEN_DATETIME = 19;
	static const int STRLEN_DATETIME2 = 14;

    std::string currentdatetime()
    {
        char szTime[STRLEN_DATETIME+1] = {0};

#if defined(_WIN32)
        time_t _Time = time(0);
        tm _Tm = {0};

        errno_t err = localtime_s(&_Tm,&_Time);
        if (!err)
        { 
            sprintf_s(szTime,"%04d-%02d-%02d %02d:%02d:%02d",
                _Tm.tm_year+1900,_Tm.tm_mon+1,_Tm.tm_mday,
                _Tm.tm_hour,_Tm.tm_min,_Tm.tm_sec);
        }

#else //_WIN32
        time_t _Time = time(0);
        tm* ptm = localtime(&_Time);
        if (ptm)
        {
            sprintf(szTime,"%04d-%02d-%02d %02d:%02d:%02d",
                ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,
                ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
        }
#endif //_WIN32

        return szTime;
    }

	std::string currentdatetime2()
	{
		char szTime[STRLEN_DATETIME2+1] = {0};

#if defined(_WIN32)
		time_t _Time = time(0);
		tm _Tm = {0};

		errno_t err = localtime_s(&_Tm,&_Time);
		if (!err)
		{ 
			sprintf_s(szTime,"%04d%02d%02d%02d%02d%02d",
				_Tm.tm_year+1900,_Tm.tm_mon+1,_Tm.tm_mday,
				_Tm.tm_hour,_Tm.tm_min,_Tm.tm_sec);
		}

#else //_WIN32
		time_t _Time = time(0);
		tm* ptm = localtime(&_Time);
		if (ptm)
		{
			sprintf(szTime,"%04d%02d%02d%02d%02d%02d",
				ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,
				ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
		}
#endif //_WIN32

		return szTime;
	}

    int currenttime()
    {
        return (int)time(0);
    }

    int diff(int t1, int t2/* = 0*/)
    {
        if (t2 == 0)
        {
            t2 = cttk::datetime::currenttime();
        }
        return (int)difftime(t2, t1);
    }

} // namespace datetime

//==============================================================================

namespace str
{
    char lowcase(const char x)
    {
        return (char)((x<'A'||x>'Z')?x:x+'a'-'A');
    }

    char upcase(const char x)
    {
        return (char)((x<'a'||x>'z')?x:x+'A'-'a');
    }

    char* lowcase(char* s)
    {
        if(s) for (char* ptr = s; *ptr; ++ptr) *ptr = cttk::str::lowcase(*ptr);

        return s;
    }

    char* upcase(char* s)
    {
        if(s) for (char* ptr = s; *ptr; ++ptr) *ptr = cttk::str::upcase(*ptr);
        return s;
    }

    std::string& lowcase(std::string& rs)
    {
        int len = (int)rs.length();
        if (len > 0)
        {
            std::string s; s.clear();
            for (int i = 0; i < len; ++i) s.push_back(cttk::str::lowcase(rs[i]));
            rs = s;
        }
        return rs;
    }

    std::string& upcase(std::string& rs)
    {
        int len = (int)rs.length();
        if (len > 0)
        {
            std::string s; s.clear();
            for (int i = 0; i < len; ++i) s.push_back(cttk::str::upcase(rs[i]));
            rs = s;
        }
        return rs;
    }

    int nocasecmp(const char* s1, const char* s2, const int l)
    {
        if (!l) return 0;
        if (!s1) return s2?-1:0;
        const char *ns1 = s1, *ns2 = s2;
        int k, diff = 0; for (k = 0; k<l && !(diff = cttk::str::lowcase(*ns1) - cttk::str::lowcase(*ns2)); ++k) { ++ns1; ++ns2; }
        return k!=l?diff:0;
    }

    int nocasecmp(const char* s1, const char* s2)
    {
        if (!s1) return s2?-1:0;
        const unsigned int l1 = (int)strlen(s1), l2 = (int)strlen(s2);
        return cttk::str::nocasecmp(s1,s2,1+(l1<l2?l1:l2));
    }

    int nocasecmp(const std::string& rs1, const std::string& rs2)
    {
        return nocasecmp(rs1.c_str(), rs2.c_str());
    }

    void trimleft(std::string& src, const std::string& trimed/* = " "*/) 
    { 
        if (src.empty())return; 

        while(trimed.find(src[0]) != std::string::npos) src.erase(0,1); 
    }

    void trimright(std::string& src, const std::string& trimed/* = " "*/)
    { 
        if (src.empty()) return; 

        while(trimed.find(src[src.length()-1]) != std::string::npos) src.erase(src.length()-1);
    }

    void trim(std::string& src, const std::string& trimed/* = " "*/)
    { 
        trimleft(src, trimed); 
        trimright(src, trimed); 
    }

    int split(std::vector<std::string>& ar, const std::string& src, const std::string& delimiter, bool filter)
    {
        std::string sTemp = src;
        int nDelimiter = (int)delimiter.length();
        int iPos = 0;

        for(;;)
        {
            iPos = (int)sTemp.find(delimiter);
            
            if (iPos < 0) break;
            else if (iPos > 0) ar.push_back(sTemp.substr(0,iPos));
            else if (iPos == 0 && !filter) ar.push_back("");

            sTemp = sTemp.substr(iPos+nDelimiter);
        }

        if(sTemp.length() > 0 || !filter)
        {
            ar.push_back(sTemp);
        }

        return (int)ar.size();
    }

    int cttk_vsnprintf(char* buffer, int len, const char* fmt, va_list ap)
    {
#ifdef _MSC_VER
        int r = _vsnprintf(buffer, len, fmt, ap);
        buffer[len-1] = '\0';

        return r;

//         if (r >= 0 && r < len) return r;
//         else return _vscprintf(fmt, ap);
#else
        int r = vsnprintf(buffer, len, fmt, ap);
        buffer[len-1] = '\0';
        return r;
#endif
    }

    std::string assemble(const char* fmt, ...)
    {
        char szBuffer[BUFFER_LEN] = {0};

        va_list _arg;
        va_start(_arg , fmt);
        cttk_vsnprintf(szBuffer, BUFFER_LEN, fmt , _arg);
        va_end(_arg);

        return szBuffer;
    }

    std::string tostr(int n)
    {
        char szBuffer[BUFFER_LEN_SHORT] = {0};

#if defined(_WIN32)
        sprintf_s(szBuffer,"%d",n);
#else //_WIN32
        sprintf(szBuffer,"%d",n);
#endif //_WIN32
        return szBuffer;
    }

    std::string ltostr(long l)
    {
        char szBuffer[BUFFER_LEN_SHORT] = {0};

#if defined(_WIN32)
        sprintf_s(szBuffer,"%ld",l);
#else //_WIN32
        sprintf(szBuffer,"%ld",l);
#endif //_WIN32
        return szBuffer;
    }

    bool toint(const char* pstr, int& n)
    {
        bool bRet = false;

        do 
        {
            if (!pstr) break;

            if (strlen(pstr) <= 0) break;

            n = atoi(pstr);
            std::ostringstream ostr;
            ostr << n;
            if (strcmp(pstr, ostr.str().c_str()) != 0) break;

            bRet = true;
        } while (0);

        return bRet;
    }

} // namespace str

namespace filedir
{
	bool exist(const std::string& rsfile)
	{
#if defined(_WIN32)
		return (_access(rsfile.c_str(), 0) == 0);
#else //_WIN32
		return (access(rsfile.c_str(), R_OK) == 0);
#endif //_WIN32
	}

	bool mvfile(const std::string& rsfrom, const std::string& rsto)
	{
#if defined(_WIN32)
		return (rename(rsfrom.c_str(), rsto.c_str()) == 0);
#else //_WIN32
		return (rename(rsfrom.c_str(), rsto.c_str()) == 0);
#endif //_WIN32
	}

    int listdir(const std::string& rsdir, const std::string& rsmacthing, std::vector<std::string>& rvfiles)
    {
		int nRet = -1;

		do 
		{
			if (rsdir.length() <= 0) break;

			rvfiles.clear();

#if defined(_WIN32)
			BOOL bRet = TRUE;
			WIN32_FIND_DATA fd;

			std::string sMatching = rsdir + FILE_SEPARATOR + rsmacthing;

			HANDLE hFile = FindFirstFile(sMatching.c_str(), &fd);
			while((hFile != INVALID_HANDLE_VALUE) && bRet)
			{
				if( (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
				{
					std::string sFile = rsdir + FILE_SEPARATOR + fd.cFileName;
					rvfiles.push_back(sFile);
				}
				bRet = FindNextFile(hFile, &fd);
			}

			FindClose(hFile);
#else //_WIN32
			DIR* dir = NULL;
			struct dirent* pDirent = NULL;

			struct stat statbuf;

			if((dir = opendir(rsdir.c_str())) == NULL) break;

			while((pDirent = readdir(dir)) != NULL)
			{
				if(*pDirent->d_name != '.')
				{
					if (fnmatch(rsmacthing.c_str(), pDirent->d_name, FNM_PATHNAME|FNM_PERIOD) != 0) continue;

					std::string sFile = rsdir + FILE_SEPARATOR + pDirent->d_name;
					stat(sFile.c_str(), &statbuf);
					if (S_ISDIR(statbuf.st_mode)) continue;

					rvfiles.push_back(sFile);
				}
			}

			closedir(dir);
#endif //_WIN32

			nRet = (int)rvfiles.size();
		} while (0);

        return nRet;
    }

} // namespace filedir

//==============================================================================

} // namespace cttk

