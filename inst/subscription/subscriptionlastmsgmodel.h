#ifndef SUBSCRIPTIONLASTMSGMODEL_H
#define SUBSCRIPTIONLASTMSGMODEL_H

#include <QStandardItemModel>
#include <QScopedPointer>
#include <QMap>
#include <QList>

namespace DB {
	class SubscriptionDB;
};
class SubscriptionMsg;

class SubscriptionLastMsgModel : public QStandardItemModel
{
	Q_OBJECT

public:
	class LastMsgItem
	{
	public:
		LastMsgItem() : send(false) {}
		QString subId;
		QString subName;
		QString lastBody;
		QString lastTime;
		QString innerId;
		bool    send;
	};

public:
	SubscriptionLastMsgModel(QObject *parent = 0);
	~SubscriptionLastMsgModel();

	void readFromDB();
	void release();

	void addLastMsg(const SubscriptionMsg &msg, const QString &name);
	void removeLastMsg(const QString &subscriptionId);
	LastMsgItem latestMsg() const;
	QStringList allSubscriptionIds() const;

	static QStandardItem *lastMsg2ModelItem(const LastMsgItem &lastMsg);
	static LastMsgItem modelItem2LastMsg(const QStandardItem &item);

Q_SIGNALS:
	void subscriptionLastMsgChanged();

private:
	void setLastMsgs(const QList<LastMsgItem> &lastMsgs);

private:
	QScopedPointer<DB::SubscriptionDB> m_subscriptionDB;
	QMap<QString, QStandardItem *>     m_items;
};

#endif // SUBSCRIPTIONLASTMSGMODEL_H
