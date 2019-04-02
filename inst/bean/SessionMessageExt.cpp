#include "SessionMessageExt.h"

namespace bean
{
	QVariantMap SessionMessageExt::toJson(const QVariantMap & /*data*/)
	{
		QVariantMap vmap;
		vmap["type"] = "session";
		return vmap;
	}

	QDomElement SessionMessageExt::toXml(const QVariantMap & /*data*/, QDomDocument &doc)
	{
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "session");
		return elem;
	}

	QString SessionMessageExt::toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText)
	{
		Q_UNUSED(data);
		Q_UNUSED(isSend);
		Q_UNUSED(username);
		return bodyText;
	}
}

