#ifndef __NET_UTIL_H__
#define __NET_UTIL_H__
#include <QString>
#include "bean/HostInfo.h"

class NetUtil
{
public:
    // addrs 必须是以字符'/'分割的地址组，‘/’前面的是外网，'/'后面的是内网。根据配置返回一个合适的地址。
    static bean::HostInfo getRightAddress(const QString &addrs);

private:
    NetUtil() {}
    ~NetUtil() {}
};



#endif //__NET_UTIL_H__
