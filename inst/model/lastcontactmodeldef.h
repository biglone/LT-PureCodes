#ifndef LASTCONTACTMODELDEF_H
#define LASTCONTACTMODELDEF_H

#include <QStandardItemModel>
#include "bean/MessageBody.h"
#include "lastcontactitemdef.h"
#include <QScopedPointer>
#include <QTimer>

class QSortFilterProxyModel;
class ModelManager;
class SubscriptionMsg;
namespace DB {class LastContactDB;}

class LastContactModel : public QStandardItemModel
{
	Q_OBJECT

public:
	LastContactModel(ModelManager *parent);
	~LastContactModel();

public:
	bool init();
	void release();

	void appendMsg(const bean::MessageBody &sMsg);

	void replaceMsg(const bean::MessageBody &msg, const QString &stamp);

	void appendMultiSendMsg(const bean::MessageBody &sMsg, const QString &id, const QStringList &members);

	QString multiSendMsgId(const QStringList &members, bool &newCreated) const;

	QSortFilterProxyModel *proxyModel() const;

	LastContactItem *nodeFromProxyIndex(const QModelIndex &proxyIndex);

	LastContactItem *nodeFromRow(int row);

	bool containsContact(const QString &id);

	bool containsMucGroup(const QString &id);

	bool containsDiscuss(const QString &id);

	LastContactItem *contact(const QString &id) const;

	LastContactItem *mucGroup(const QString &id) const;

	LastContactItem *discuss(const QString &id) const;

	QStringList allContactIds() const;

	QStringList allMucGroupIds() const;

	QStringList allDiscussIds() const;

	bool hasUnreadMsg() const;

	bool changeItemName(LastContactItem::LastContactItemType itemType, const QString &id, const QString &newName);

	void checkDiscussNameChanged();

	static QString typeToString(LastContactItem::LastContactItemType itemType);
	static LastContactItem::LastContactItemType stringToType(const QString &str);

	static const char *TYPE_CHAT;
	static const char *TYPE_GROUPCHAT;
	static const char *TYPE_DISCUSS;
	static const char *TYPE_MULTISEND;

public slots:
	void onUnreadItemCountChanged();
	void onRemoveChat(const QString &id);
	void onRemoveGroupChat(const QString &id);
	void onRemoveDiscuss(const QString &id);
	void onRemoveMultiSend(const QString &id);
	void onRemoveAllRecords();
	void onMultiSendMemberChanged(const QString &id, const QString &name, const QStringList &members);
	void onSubscriptionLastMsgChanged();

signals:
	void unreadMsgRecordRemoved();

private:
	bool readData();
	void writeData(const LastContactItem &item);
	void removeRecord(const QString &id, LastContactItem::LastContactItemType itemType);

	static QString makeKey(LastContactItem::LastContactItemType itemType, const QString &id);

	void cacheData(const LastContactItem &item);
	bool commit(const LastContactItem &item);

private slots:
	void commitAll();
	
private:
	QMap<QString, LastContactItem *>     m_records;
	bool                                 m_bInit;
	ModelManager*                        m_pModelManager;
	QScopedPointer<DB::LastContactDB>    m_pContactDB;
	QSortFilterProxyModel*               m_pProxyModel;

	QMap<QString, LastContactItem *>     m_writeCache;
	QTimer                               m_writeTimer;
};

#endif // LASTCONTACTMODELDEF_H
