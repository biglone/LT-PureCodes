#include <QObject>
#include "TipMessageExt.h"

namespace bean
{
	QVariantMap TipMessageExt::toJson(const QVariantMap &data)
	{
		QVariantMap vmap;
		vmap["type"] = "tip";
		vmap["level"] = data.value("level").toString();
		vmap["action"] = data.value("action").toString();
		vmap["param"] = data.value("param").toString();
		return vmap;
	}

	QDomElement TipMessageExt::toXml(const QVariantMap &data, QDomDocument &doc)
	{
		QDomElement elem = doc.createElement("ext");
		elem.setAttribute("type", "tip");
		elem.setAttribute("level", data.value("level").toString());
		elem.setAttribute("action", data.value("action").toString());
		elem.setAttribute("param", data.value("param").toString());
		return elem;
	}

	QString TipMessageExt::toText(const QVariantMap & /*data*/, bool /*isSend*/, const QString & /*username*/, const QString &bodyText)
	{
		return bodyText;
	}

}

