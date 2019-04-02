///@file Base.h ��������.

#ifndef _CTTK_BASE_H_INCLUDED
#define _CTTK_BASE_H_INCLUDED

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include <string>
#include <vector>

/// ��ȫ�ͷ�(����)
#define SAFE_DELETE(_x) do{delete(_x);(_x)= NULL;}while(0)
/// ��ȫ�ͷ�(����)
#define SAFE_DELETE_ARRAY(_x) do{delete [](_x);(_x) = NULL;}while(0)

/// string���Ͱ�ȫ��ֵ
#define SAFE_ASSIGN_STRING(str,ptr) do {if (ptr) str = ptr;} while(0)

/// �������
#define DEBUG_OUT(format, ...) cttk::dbg::print(format,__VA_ARGS__)

/// �ļ��ָ���
#if defined(_WIN32)
#  define FILE_SEPARATOR "\\"
#else //_WIN32
#  define FILE_SEPARATOR "/"
#endif //_WIN32

///@brief ���߿����ֿռ�.
namespace cttk
{
    static const int BUFFER_LEN         = 1024*2;
    static const int BUFFER_LEN_SHORT   = 100;

    namespace dbg
    {
        /// �����������
        void show(bool b = true);

        /// ��ӡ
        void print(const char* fmt, ...);

    } // namespace dbg

    namespace sys
    {
        /// �̹߳���:��
        void sleep(int s);

        /// �̹߳���:����
        void usleep(int ms);
    }

    namespace datetime
    {
        /// ��ȡ��ǰʱ��:yyyy-mm-dd HH-MM-SS
        std::string currentdatetime();

		/// ��ȡ��ǰʱ��:yyyymmddHHMMSS
		std::string currentdatetime2();

        /// ��ǰʱ��
        int currenttime();
        
        /// ʱ��Ƚ�
        int diff(int t1, int t2 = 0);
    }

    namespace str
    {
        /// �ַ�ת����Сд
        char lowcase(const char x);
        /// �ַ�ת���ɴ�д
        char upcase(const char x);

        /// �ַ���ת����Сд
        char* lowcase(char* s);
        /// �ַ���ת���ɴ�д
        char* upcase(char* s);

        /// �ַ���(string)ת����Сд
        std::string& lowcase(std::string& rs);
        /// �ַ���(string)ת���ɴ�д
        std::string& upcase(std::string& rs);

        /// ���Դ�Сд���ַ����Ƚ�(ָ������)
        int nocasecmp(const char* s1, const char* s2, const int l);
        /// ���Դ�Сд���ַ����Ƚ�
        int nocasecmp(const char* s1, const char* s2);
        /// ���Դ�Сд���ַ���(string)�Ƚ�
        int nocasecmp(const std::string& rs1, const std::string& rs2);

        /// �ַ�������(��)
        void trimleft(std::string& src, const std::string& trimed = " ");
        /// �ַ�������(��)
        void trimright(std::string& src, const std::string& trimed = " ");
        /// �ַ�������
        void trim(std::string& src, const std::string& trimed = " ");

        /// �ַ����ָ�,filter:�Ƿ���˿��ֶ�
        int split(std::vector<std::string>& ar, const std::string& src, const std::string& delimiter, bool filter = true);

        /// �ַ���ƴװ
        int cttk_vsnprintf(char* buffer, int len, const char* fmt, va_list ap);
        std::string assemble(const char* fmt, ...);

        /// int => string
        std::string tostr(int n);

        /// long => string
        std::string ltostr(long l);

        /// string => int
        bool toint(const char* pstr, int& n);
    } // namespace str

    namespace filedir
    {
		/// �ж��ļ��Ƿ����
		bool exist(const std::string& rsfile);

		/// �������ļ�
		bool mvfile(const std::string& rsfrom, const std::string& rsto);

        /// �г�Ŀ¼��ָ���ļ�
        int listdir(const std::string& rsdir, const std::string& rsfile, std::vector<std::string>& rvfiles);
    } // namespace filedir

} // namespace cttk

#endif //_CTTK_BASE_H_INCLUDED
