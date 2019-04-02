#ifndef _SESSIONMESSAGEEXT_H_
#define _SESSIONMESSAGEEXT_H_

#include "MessageExtBase.h"

namespace bean
{
	class SessionMessageExt
	{
	public:
		static QVariantMap toJson(const QVariantMap &data);
		static QDomElement toXml(const QVariantMap &data, QDomDocument &doc);
		static QString toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText);
	};
}
#endif //_SESSIONMESSAGEEXT_H_
