#ifndef _ATMESSAGEEXT_H_
#define _ATMESSAGEEXT_H_

#include "MessageExtBase.h"

namespace bean
{
	class AtMessageExt
	{
	public:
		static QVariantMap toJson(const QVariantMap &data);
		static QDomElement toXml(const QVariantMap &data, QDomDocument &doc);
		static QString toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText);
	};
}
#endif //_ATMESSAGEEXT_H_
