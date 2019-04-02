#ifndef _MESSAGEEXTBASE_H_
#define _MESSAGEEXTBASE_H_

#include <QString>
#include <QVariantMap>
#include <QSharedData>
#include <QDomElement>
#include "bean.h"

namespace bean
{
	class MessageExtBase : public QSharedData
	{
	public:
		MessageExtBase();
		MessageExtBase(bean::MessageExtType type);
		MessageExtBase(const MessageExtBase &other);

		QVariantMap toJson();
		QDomElement toXml(QDomDocument &doc);
		QString toText(bool isSend, const QString &username, const QString &bodyText);

		MessageExtType type;
		QVariantMap    data;
	};
}

#endif //_MESSAGEEXTBASE_H_
