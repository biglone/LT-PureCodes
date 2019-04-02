#include "JsonParser.h"
#include "qt-json/json.h"

QVariant JsonParser::parse(const QByteArray &data, bool &err, QString &errMsg, int okCode /*= 0*/, int *retCode /*= 0*/)
{
	QVariant v;
	err = true;
	errMsg = "";
	if (retCode)
		*retCode = 0;

	do {
		if (data.isEmpty())
		{
			errMsg = QObject::tr("data error");
			break;
		}

		bool parseOK = false;
		QVariantMap vmap = QtJson::parse(QString::fromUtf8(data), parseOK).toMap();
		if (!parseOK)
		{
			errMsg = QObject::tr("data error");
			break;
		}

		int ret = vmap["ret"].toInt();
		QString msg = vmap["msg"].toString();
		if (retCode)
			*retCode = ret;
		if (ret != okCode)
		{
			errMsg = QString("%1:%2").arg(msg).arg(ret);
			break;
		}

		v = vmap["datas"];
		err = false;
	} while (0);

	return v;
}