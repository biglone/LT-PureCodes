#include <QObject>
#include "ShareMessageExt.h"

namespace bean
{
	QVariantMap ShareMessageExt::toJson(const QVariantMap &data)
	{
		QVariantMap vmap;
		vmap["type"] = "share";
		vmap["shareurl"] = data.value("shareurl").toString();

		return vmap;
	}

	QDomElement ShareMessageExt::toXml(const QVariantMap &data, QDomDocument &doc)
	{
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "share");
		elem.setAttribute("url", data.value("shareurl").toString());
		return elem;
	}

	QString ShareMessageExt::toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText)
	{
		Q_UNUSED(isSend);
		Q_UNUSED(username);
		QString url = data.value("shareurl").toString();

		if (url.isEmpty())
			return bodyText;
		else
			return QObject::tr("share message:")+bodyText;
	}

}

