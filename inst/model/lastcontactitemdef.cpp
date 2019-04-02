#include "lastcontactitemdef.h"

LastContactItem::LastContactItem(LastContactItemType itemType, const QString &id, const QString &name)
{
	setItemType(itemType);
	setItemId(id);
	setItemName(name);
	setLastBody(QString());
	setLastTime(QString());
	setAttachment(0);
	setUnreadMsgCount(0);
	setMultiSendMembers(QStringList());
	setTimeStamp(QString());
	setSendUid(QString());
}

LastContactItem::LastContactItem(const LastContactItem &other)
{
	setItemType(other.itemType());
	setItemId(other.itemId());
	setItemName(other.itemName());
	setLastBody(other.lastBody());
	setLastTime(other.lastTime());
	setAttachment(other.attachment());
	setUnreadMsgCount(other.unreadMsgCount());
	setMultiSendMembers(other.multiSendMemebers());
	setTimeStamp(other.timeStamp());
	setSendUid(other.sendUid());
}

LastContactItem::~LastContactItem()
{
}

void LastContactItem::setItemType(LastContactItemType itemType)
{
	setData(itemType, LastContactTypeRole);
}

LastContactItem::LastContactItemType LastContactItem::itemType() const
{
	return (LastContactItemType)(data(LastContactTypeRole).toInt());
}

void LastContactItem::setItemId(const QString &id)
{
	setData(id, LastContactIdRole);
}

QString LastContactItem::itemId() const
{
	return data(LastContactIdRole).toString();
}

void LastContactItem::setItemName(const QString &name)
{
	setData(name, LastContactNameRole);
}

QString LastContactItem::itemName() const
{
	return data(LastContactNameRole).toString();
}

void LastContactItem::setLastBody(const QString &body)
{
	setData(body, LastContactBodyRole);
}

QString LastContactItem::lastBody() const
{
	return data(LastContactBodyRole).toString();
}

void LastContactItem::setLastTime(const QString &lastTime)
{
	setData(lastTime, LastContactTimeRole);
}

QString LastContactItem::lastTime() const
{
	return data(LastContactTimeRole).toString();
}

void LastContactItem::setAttachment(int att)
{
	setData(att, LastContactAttRole);
}

int LastContactItem::attachment() const
{
	return data(LastContactAttRole).toInt();
}

void LastContactItem::setUnreadMsgCount(int count)
{
	setData(count, LastContactMsgCountRole);
}

int LastContactItem::unreadMsgCount() const
{
	return data(LastContactMsgCountRole).toInt();
}

void LastContactItem::setMultiSendMembers(const QStringList &members)
{
	setData(members, LastContactMultiSendMembers);
}

QStringList LastContactItem::multiSendMemebers() const
{
	return data(LastContactMultiSendMembers).toStringList();
}

void LastContactItem::setTimeStamp(const QString &ts)
{
	setData(ts, LastContactTimeStampRole);
}

QString LastContactItem::timeStamp() const
{
	return data(LastContactTimeStampRole).toString();
}

void LastContactItem::setSendUid(const QString &uid)
{
	setData(uid, LastContactSendUid);
}

QString LastContactItem::sendUid() const
{
	return data(LastContactSendUid).toString();
}