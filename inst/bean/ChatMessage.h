#ifndef _CHATMESSAGE_H_
#define _CHATMESSAGE_H_

#include "MessageBase.h"

namespace bean
{
	class ChatMessage
	{
	public:
		ChatMessage(MessageBase &data);

	public:
		QVariantMap toJson();          // to html display

		QVariantMap toMessageDBMap();  // to messages database

		QString toMessageXml();

	private:
		QString toId() const;

	private:
		MessageBase &m_data;
	};
}
#endif //_CHATMESSAGE_H_
