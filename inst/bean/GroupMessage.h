#ifndef _GROUPMESSAGE_H_
#define _GROUPMESSAGE_H_

#include "MessageBase.h"

namespace bean
{
	class GroupMessage
	{
	public:
		GroupMessage(MessageBase &data);

	public:
		QVariantMap toJson();          // to html display

		QVariantMap toMessageDBMap();  // to messages database

		QString toMessageXml();

	private:
		MessageBase &m_data;
	};
}
#endif //_GROUPMESSAGE_H_
