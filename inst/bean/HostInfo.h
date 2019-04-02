#ifndef _HOST_INFO_H_
#define _HOST_INFO_H_
#include <QHostAddress>
#include <QSharedDataPointer>

namespace bean
{
    class HostInfoData;
    class HostInfo
    {
    public:
        HostInfo();
        HostInfo(const HostInfo &other);
        virtual ~HostInfo();

        HostInfo& operator=(const HostInfo &other);
        bool operator==(const HostInfo &other);

        bool isValid() const;

        void setIp(const QHostAddress &ip);
        void setPort(int port);
        QHostAddress ip() const;
        int port() const;

    private:
        QSharedDataPointer<HostInfoData> d;
    };
}

inline bool operator==( const bean::HostInfo &a,const bean::HostInfo &b )
{
    return a.ip() == b.ip() && a.port() == b.port();
}

inline uint qHash( const bean::HostInfo &key )
{
    return qHash(QString("%1:%2").arg(key.ip().toString()).arg(key.port()));
}
#endif
