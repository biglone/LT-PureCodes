#include <QObject>
#include "ShakeMessageExt.h"

namespace bean
{
	QVariantMap ShakeMessageExt::toJson(const QVariantMap & /*data*/)
	{
		QVariantMap vmap;
		vmap["type"] = "shake";
		return vmap;
	}

	QDomElement ShakeMessageExt::toXml(const QVariantMap & /*data*/, QDomDocument &doc)
	{
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "shake");
		
		return elem;
	}

	QString ShakeMessageExt::toText(const QVariantMap & /*data*/, bool isSend, const QString &username, const QString & /*bodyText*/)
	{
		if (isSend)
		{
			return QObject::tr("You sent a shake message.");
		}
		else
		{
			return QObject::tr("%1 sent a shake message.").arg(username);
		}
	}

}

