#include "InterphoneMessageExt.h"

namespace bean
{
	QVariantMap InterphoneMessageExt::toJson(const QVariantMap & /*data*/)
	{
		QVariantMap vmap;
		vmap["type"] = "interphone";
		return vmap;
	}

	QDomElement InterphoneMessageExt::toXml(const QVariantMap & /*data*/, QDomDocument &doc)
	{
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "interphone");
		return elem;
	}

	QString InterphoneMessageExt::toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText)
	{
		Q_UNUSED(data);
		Q_UNUSED(isSend);
		Q_UNUSED(username);
		return bodyText;
	}
}

