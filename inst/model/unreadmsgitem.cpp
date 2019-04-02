#include "unreadmsgitem.h"

UnreadMsgItem::UnreadMsgItem() : QStandardItem()
{
	setId(QString());
	setMsgType(bean::Message_Chat);
	setName(QString());
	setIgnoreBefore(false);
	setMsgCount(m_msgs.count());
}

UnreadMsgItem::~UnreadMsgItem()
{
	m_msgs.clear();
}

void UnreadMsgItem::setId(const QString &id)
{
	setData(id, IdRole);
}

QString UnreadMsgItem::id() const
{
	return data(IdRole).toString();
}

void UnreadMsgItem::setName(const QString &name)
{
	setData(name, NameRole);
}

QString UnreadMsgItem::name() const
{
	return data(NameRole).toString();
}

void UnreadMsgItem::setMsgType(bean::MessageType msgType)
{
	setData(msgType, MsgTypeRole);
}

bean::MessageType UnreadMsgItem::msgType() const
{
	return (bean::MessageType)data(MsgTypeRole).toInt();
}

void UnreadMsgItem::insertMsgToTop(const bean::MessageBody &msg)
{
	m_msgs.insert(0, msg);
	setMsgCount(m_msgs.count());
}

void UnreadMsgItem::appendMsg(const bean::MessageBody &msg)
{
	m_msgs.append(msg);
	setMsgCount(m_msgs.count());
	setIgnoreBefore(false);
}

QList<bean::MessageBody> UnreadMsgItem::msgs() const
{
	return m_msgs;
}

void UnreadMsgItem::setMsgs(const QList<bean::MessageBody> &msgs)
{
	m_msgs = msgs;
	setMsgCount(m_msgs.count());
}

void UnreadMsgItem::setIgnoreBefore(bool bIgnore)
{
	setData(bIgnore, IgnoreBeforeRole);
}

bool UnreadMsgItem::isIgnoreBefore() const
{
	return data(IgnoreBeforeRole).toBool();
}

int UnreadMsgItem::msgCount() const
{
	int count = 0;
	if (m_msgs.count() >= 1 && isIgnoreBefore())
	{
		bean::MessageBody msg = m_msgs[m_msgs.count()-1];
		bool showMsg = msg.ext().data(bean::EXT_DATA_LASTCONTACT_NAME, true).toBool();
		if (showMsg)
		{
			count = m_msgs.count();
		}
		else
		{
			int showMsgCount = 0;
			for (int i = 0; i < m_msgs.count()-1; ++i)
			{
				msg = m_msgs[i];
				showMsg = msg.ext().data(bean::EXT_DATA_LASTCONTACT_NAME, true).toBool();
				if (showMsg)
					++showMsgCount;
			}
			if (showMsgCount > 0)
				count = m_msgs.count();
		}
	}
	else
	{
		count = m_msgs.count();
	}
	return count;
}

void UnreadMsgItem::setMsgCount(int count)
{
	setData(count, CountRole);
}