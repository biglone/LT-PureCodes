#ifndef _SHAREMESSAGEEXT_H_
#define _SHAREMESSAGEEXT_H_

#include "MessageExtBase.h"

namespace bean
{
	class ShareMessageExt
	{
	public:
		static QVariantMap toJson(const QVariantMap &data);
		static QDomElement toXml(const QVariantMap &data, QDomDocument &doc);
		static QString toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText);
	};
}
#endif //_SHAREMESSAGEEXT_H_
