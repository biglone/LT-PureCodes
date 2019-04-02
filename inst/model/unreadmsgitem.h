#ifndef UNREADMSGITEM_H
#define UNREADMSGITEM_H

#include <QStandardItem>
#include "bean/MessageBody.h"
#include <QList>
#include "bean/bean.h"

class UnreadMsgItem : public QStandardItem
{
public:
	enum UnreadMsgItemRoleType
	{
		IdRole = Qt::UserRole + 1,
		MsgTypeRole,
		NameRole,
		CountRole,
		IgnoreBeforeRole
	};

public:
	UnreadMsgItem();
	~UnreadMsgItem();

	void setId(const QString &id);
	QString id() const;

	void setMsgType(bean::MessageType msgType);
	bean::MessageType msgType() const;

	void setName(const QString &name);
	QString name() const;

	void insertMsgToTop(const bean::MessageBody &msg);
	void appendMsg(const bean::MessageBody &msg);
	QList<bean::MessageBody> msgs() const;
	void setMsgs(const QList<bean::MessageBody> &msgs);

	void setIgnoreBefore(bool bIgnore);
	bool isIgnoreBefore() const;

	int msgCount() const;

private:
	void setMsgCount(int count);

private:
	QList<bean::MessageBody> m_msgs;
};

#endif // UNREADMSGITEM_H
