#include "SecretMessageExt.h"

namespace bean
{
	QVariantMap SecretMessageExt::toJson(const QVariantMap & /*data*/)
	{
		QVariantMap vmap;
		vmap["type"] = "secret";
		return vmap;
	}

	QDomElement SecretMessageExt::toXml(const QVariantMap & /*data*/, QDomDocument &doc)
	{
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "secret");
		return elem;
	}

	QString SecretMessageExt::toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText)
	{
		Q_UNUSED(data);
		Q_UNUSED(isSend);
		Q_UNUSED(username);
		Q_UNUSED(bodyText);
		return QObject::tr("[Message]");
	}
}

