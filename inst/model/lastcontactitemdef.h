#ifndef LASTCONTACTITEMDEF_H
#define LASTCONTACTITEMDEF_H

#include <QStandardItem>

// class LastContactItem
// represent a item in the last contact model
class LastContactItem : public QStandardItem
{
public:
	enum LastContactItemRole
	{
		LastContactTypeRole = Qt::UserRole+1,
		LastContactIdRole,
		LastContactNameRole,
		LastContactBodyRole,
		LastContactTimeRole,
		LastContactAttRole,
		LastContactMsgCountRole,
		LastContactMultiSendMembers,
		LastContactInterphoneStateRole,
		LastContactTimeStampRole,
		LastContactSendUid
	};

	enum LastContactItemType
	{
		LastContactTypeContact,
		LastContactTypeGroupMuc,
		LastContactTypeDiscuss,
		LastContactTypeMultiSend
	};

public:
	LastContactItem(LastContactItemType itemType = LastContactTypeContact, const QString &id = QString(), const QString &name = QString());
	LastContactItem(const LastContactItem &other);
	~LastContactItem();

	void setItemType(LastContactItemType itemType);
	LastContactItemType itemType() const;

	void setItemId(const QString &id);
	QString itemId() const;

	void setItemName(const QString &name);
	QString itemName() const;

	void setLastBody(const QString &body);
	QString lastBody() const;

	void setLastTime(const QString &lastTime);
	QString lastTime() const;

	void setAttachment(int att);
	int attachment() const;

	void setUnreadMsgCount(int count);
	int unreadMsgCount() const;

	void setMultiSendMembers(const QStringList &members);
	QStringList multiSendMemebers() const;

	void setTimeStamp(const QString &ts);
	QString timeStamp() const;

	void setSendUid(const QString &uid);
	QString sendUid() const;
};

#endif // LASTCONTACTITEMDEF_H
