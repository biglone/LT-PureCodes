#ifndef UNREADMSGMODEL_H
#define UNREADMSGMODEL_H

#include <QStandardItemModel>
#include <QPair>
#include <QList>
#include <QMap>
#include "bean/MessageBody.h"

class UnreadMsgItem;
class UnreadMessageSortFilterModel;
class QListView;

class UnreadMsgModel : public QStandardItemModel
{
	Q_OBJECT

public:
	UnreadMsgModel(QObject *parent = 0);
	~UnreadMsgModel();

	UnreadMessageSortFilterModel *filterModel() const;

public:
	void insertMsg(const QString &id, 
		           bean::MessageType msgType, 
				   const bean::MessageBody &msg, 
				   bool msgAtFirst = false,
				   bool ignore = false);
	QList<bean::MessageBody> takeMsg(const QString &id, bean::MessageType msgType);
	void clean();
	bool containsMsg(const QString &id, bean::MessageType msgType);
	bool hasUnreadMsg() const;
	bool getTopUnreadMsg(QString &id, bean::MessageType &msgType) const;
	void ignoreAll();
	QList<UnreadMsgItem *> allUnreadMsgs() const;
	QList<QPair<QString, bean::MessageType>> allUnreadMsgsFrom() const;
	UnreadMsgItem * peekUnreadMsg(const QString &id, bean::MessageType msgType) const;

Q_SIGNALS:
	void lastMsgChanged(const QString &id, bean::MessageType msgType);
	void msgToken(const QString &id, bean::MessageType msgType);
	void unreadItemCountChanged();
	void preIgnoreAll();
	void preTakeMsg(const QModelIndex &sourceIndex);

private:
	QString getMsgItemKey(const QString &rsUid, bean::MessageType type) const;
	bool fromMsgItemKey(const QString &msgItemKey, QString &rsUid, bean::MessageType &type) const;

private:
	QMap<QString, UnreadMsgItem *>  m_unreadMsgItems;
	UnreadMessageSortFilterModel   *m_filterModel;
};

#endif // UNREADMSGMODEL_H
