#include "ChatMessageExt.h"

namespace bean
{
	QVariantMap ChatMessageExt::toJson(const QVariantMap &data)
	{
		Q_UNUSED(data);
		QVariantMap vmap;
		vmap["type"] = "chat";
		return vmap;
	}

	QDomElement ChatMessageExt::toXml(const QVariantMap &data, QDomDocument &doc)
	{
		Q_UNUSED(data);
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "chat");
		return elem;
	}

	QString ChatMessageExt::toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText)
	{
		Q_UNUSED(data);
		Q_UNUSED(isSend);
		Q_UNUSED(username);
		return bodyText;
	}

}