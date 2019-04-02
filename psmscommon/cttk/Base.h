///@file Base.h 基础方法.

#ifndef _CTTK_BASE_H_INCLUDED
#define _CTTK_BASE_H_INCLUDED

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include <string>
#include <vector>

/// 安全释放(数组)
#define SAFE_DELETE(_x) do{delete(_x);(_x)= NULL;}while(0)
/// 安全释放(数组)
#define SAFE_DELETE_ARRAY(_x) do{delete [](_x);(_x) = NULL;}while(0)

/// string类型安全赋值
#define SAFE_ASSIGN_STRING(str,ptr) do {if (ptr) str = ptr;} while(0)

/// 调试输出
#define DEBUG_OUT(format, ...) cttk::dbg::print(format,__VA_ARGS__)

/// 文件分隔符
#if defined(_WIN32)
#  define FILE_SEPARATOR "\\"
#else //_WIN32
#  define FILE_SEPARATOR "/"
#endif //_WIN32

///@brief 工具库名字空间.
namespace cttk
{
    static const int BUFFER_LEN         = 1024*2;
    static const int BUFFER_LEN_SHORT   = 100;

    namespace dbg
    {
        /// 设置输出开关
        void show(bool b = true);

        /// 打印
        void print(const char* fmt, ...);

    } // namespace dbg

    namespace sys
    {
        /// 线程挂起:秒
        void sleep(int s);

        /// 线程挂起:毫秒
        void usleep(int ms);
    }

    namespace datetime
    {
        /// 获取当前时间:yyyy-mm-dd HH-MM-SS
        std::string currentdatetime();

		/// 获取当前时间:yyyymmddHHMMSS
		std::string currentdatetime2();

        /// 当前时间
        int currenttime();
        
        /// 时间比较
        int diff(int t1, int t2 = 0);
    }

    namespace str
    {
        /// 字符转换成小写
        char lowcase(const char x);
        /// 字符转换成大写
        char upcase(const char x);

        /// 字符串转换成小写
        char* lowcase(char* s);
        /// 字符串转换成大写
        char* upcase(char* s);

        /// 字符串(string)转换成小写
        std::string& lowcase(std::string& rs);
        /// 字符串(string)转换成大写
        std::string& upcase(std::string& rs);

        /// 忽略大小写的字符串比较(指定长度)
        int nocasecmp(const char* s1, const char* s2, const int l);
        /// 忽略大小写的字符串比较
        int nocasecmp(const char* s1, const char* s2);
        /// 忽略大小写的字符串(string)比较
        int nocasecmp(const std::string& rs1, const std::string& rs2);

        /// 字符串清理(左)
        void trimleft(std::string& src, const std::string& trimed = " ");
        /// 字符串清理(右)
        void trimright(std::string& src, const std::string& trimed = " ");
        /// 字符串清理
        void trim(std::string& src, const std::string& trimed = " ");

        /// 字符串分割,filter:是否过滤空字段
        int split(std::vector<std::string>& ar, const std::string& src, const std::string& delimiter, bool filter = true);

        /// 字符串拼装
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
		/// 判断文件是否存在
		bool exist(const std::string& rsfile);

		/// 重命名文件
		bool mvfile(const std::string& rsfrom, const std::string& rsto);

        /// 列出目录下指定文件
        int listdir(const std::string& rsdir, const std::string& rsfile, std::vector<std::string>& rvfiles);
    } // namespace filedir

} // namespace cttk

#endif //_CTTK_BASE_H_INCLUDED
