#ifndef _INTERPHONEMESSAGEEXT_H_
#define _INTERPHONEMESSAGEEXT_H_

#include "MessageExtBase.h"

namespace bean
{
	class InterphoneMessageExt
	{
	public:
		static QVariantMap toJson(const QVariantMap &data);
		static QDomElement toXml(const QVariantMap &data, QDomDocument &doc);
		static QString toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText);
	};
}

#endif //_INTERPHONEMESSAGEEXT_H_
