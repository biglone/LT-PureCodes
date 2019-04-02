#include <QObject>
#include "AtMessageExt.h"

namespace bean
{
	QVariantMap AtMessageExt::toJson(const QVariantMap &data)
	{
		QVariantMap vmap;
		vmap["type"] = "at";
		vmap["at"] = data.value("at").toString();
		vmap["atid"] = data.value("atid").toString();
		return vmap;
	}

	QDomElement AtMessageExt::toXml(const QVariantMap &data, QDomDocument &doc)
	{
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "at");
		elem.setAttribute("at", data.value("at").toString());
		elem.setAttribute("atid", data.value("atid").toString());
		return elem;
	}

	QString AtMessageExt::toText(const QVariantMap & /*data*/, bool /*isSend*/, const QString & /*username*/, const QString &bodyText)
	{
		return bodyText;
	}

}

