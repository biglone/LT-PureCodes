#ifndef _CHATMESSAGEEXT_H_
#define _CHATMESSAGEEXT_H_

#include "MessageExtBase.h"

namespace bean
{
	class ChatMessageExt
	{ 
	public:
		static QVariantMap toJson(const QVariantMap &data);
		static QDomElement toXml(const QVariantMap &data, QDomDocument &doc);      
		static QString toText(const QVariantMap &data, bool isSend, const QString &username, const QString &bodyText);
	};
}
#endif //_CHATMESSAGEEXT_H_
