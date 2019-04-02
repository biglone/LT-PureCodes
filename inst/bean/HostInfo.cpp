#include "HostInfo.h"

namespace bean
{

    class HostInfoData : public QSharedData
    {
    public:
        HostInfoData();
        HostInfoData(const HostInfoData &other);
        virtual ~HostInfoData();

        QHostAddress ip;
        int port;

    };

    HostInfoData::HostInfoData()
        : port(-1)
    {

    }

    HostInfoData::HostInfoData( const HostInfoData &other )
    {
        ip = other.ip;
        port = other.port;
    }

    HostInfoData::~HostInfoData()
    {
    }

    //////////////////////////////////////////////////////////////////////////

    HostInfo::HostInfo()
        : d(new HostInfoData)
    {

    }

    HostInfo::HostInfo( const HostInfo &other )
        : d(other.d)
    {

    }

    HostInfo::~HostInfo()
    {
    }

    HostInfo& HostInfo::operator=( const HostInfo &other )
    {
        d = other.d;
        return (*this);
    }

    bool HostInfo::operator==( const HostInfo &other )
    {
        return d->ip == other.d->ip && d->port == other.d->port;
    }

    bool HostInfo::isValid() const
    {
        if (!d->ip.isNull() && d->port > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void HostInfo::setIp( const QHostAddress &ip )
    {
        d->ip = ip;
    }

    void HostInfo::setPort( int port )
    {
        d->port = port;
    }

    QHostAddress HostInfo::ip() const
    {
        return d->ip;
    }

    int HostInfo::port() const
    {
        return d->port;
    }
}


