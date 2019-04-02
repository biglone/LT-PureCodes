#ifndef __NET_UTIL_H__
#define __NET_UTIL_H__
#include <QString>
#include "bean/HostInfo.h"

class NetUtil
{
public:
    // addrs ���������ַ�'/'�ָ�ĵ�ַ�飬��/��ǰ�����������'/'��������������������÷���һ�����ʵĵ�ַ��
    static bean::HostInfo getRightAddress(const QString &addrs);

private:
    NetUtil() {}
    ~NetUtil() {}
};



#endif //__NET_UTIL_H__
