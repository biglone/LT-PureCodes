#include <QStringList>
#include <QSettings>
#include "settings/GlobalSettings.h"
#include "PsgManager.h"
#include "NetUtil.h"

bean::HostInfo NetUtil::getRightAddress(const QString &addrs)
{
    bean::HostInfo ret;

    do 
    {
        if (addrs.isEmpty())
            break;

        QStringList lstAddr = addrs.split("/");
        if (lstAddr.isEmpty())
        {
            break;
        }

        QString addr = lstAddr.at(0);
		GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
		QString innerName = QString(IN_ADDRESS_NAME);
        if (curConfig.netType == innerName && (lstAddr.count() > 1))
        {
            addr = lstAddr.at(1);
        }

        QStringList ipport = addr.split(":");
        if (ipport.isEmpty() || ipport.length() != 2)
        {
            break;
        }

        bool bOk = false;
        int port = ipport.at(1).toInt(&bOk);
        if (!bOk)
        {
            break;
        }

        ret.setIp(QHostAddress(ipport.at(0)));
        ret.setPort(port);

    } while (0);

    return ret;
}



