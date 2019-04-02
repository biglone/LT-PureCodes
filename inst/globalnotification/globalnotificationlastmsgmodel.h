#ifndef GLOBALNOTIFICATIONLASTMSGMODEL_H
#define GLOBALNOTIFICATIONLASTMSGMODEL_H

#include <QStandardItemModel>
#include <QScopedPointer>
#include <QMap>
#include <QList>

namespace DB {
	class GlobalNotificationDB;
};
class GlobalNotificationMsg;

class GlobalNotificationLastMsgModel : public QStandardItemModel
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
	GlobalNotificationLastMsgModel(QObject *parent = 0);
	~GlobalNotificationLastMsgModel();

	void readFromDB();
	void release();

	void addLastMsg(const GlobalNotificationMsg &msg, const QString &name);
	void removeLastMsg(const QString &globalNotificationId);
	LastMsgItem latestMsg() const;
	QStringList allGlobalNotificationIds() const;

	static QStandardItem *lastMsg2ModelItem(const LastMsgItem &lastMsg);
	static LastMsgItem modelItem2LastMsg(const QStandardItem &item);

Q_SIGNALS:
	void globalNotificationLastMsgChanged();

private:
	void setLastMsgs(const QList<LastMsgItem> &lastMsgs);

private:
	QScopedPointer<DB::GlobalNotificationDB> m_globalNotificationDB;
	QMap<QString, QStandardItem *>     m_items;
};

#endif // GLOBALNOTIFICATIONLASTMSGMODEL_H
