#ifndef _DISCUSSMESSAGE_H_
#define _DISCUSSMESSAGE_H_

#include "MessageBase.h"

namespace bean
{
	class DiscussMessage
	{
	public:
		DiscussMessage(MessageBase &data);

	public:
		QVariantMap toJson();          // to html display

		QVariantMap toMessageDBMap();  // to messages database

		QString toMessageXml();

	private:
		MessageBase &m_data;
	};
}
#endif //_DISCUSSMESSAGE_H_

