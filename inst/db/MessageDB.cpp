#include "MessageDB.h"
#include "ChatMessageDB.h"
#include "GroupMessageDB.h"
#include "DiscussMessageDB.h"
#include "ComponentMessageDB.h"

namespace DB
{
	MessageDB *MessageDBFactory::createMessageDB(bean::MessageType msgType, const QString &connSuffix)
	{
		MessageDB *messageDB = 0;
		if (msgType == bean::Message_Chat)
			messageDB = new ChatMessageDB(connSuffix);
		else if (msgType == bean::Message_GroupChat)
			messageDB = new GroupMessageDB(connSuffix);
		else if (msgType == bean::Message_DiscussChat)
			messageDB = new DiscussMessageDB(connSuffix);

		return messageDB;
	}
}