#ifndef ADDFRIENDMANAGER_H
#define ADDFRIENDMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>

class HttpPool;

class AddFriendManager : public QObject
{
	Q_OBJECT

public:
	enum Action
	{
		Request,
		Accept,
		Refuse
	};

	class Item
	{
	public:
		Item() : m_action(AddFriendManager::Request), m_status(0), m_read(0), m_delete(false) {}
		Item(const QString &fromId, const QString &fromName, const QString &toId, const QString &toName,
			const QString &sId, AddFriendManager::Action action, 
			const QString &message, const QString &time, const QString &group, int status, int read)
		{
			m_fromId = fromId;
			m_fromName = fromName;
			m_toId = toId;
			m_toName = toName;
			m_sId = sId;
			m_action = action;
			m_message = message;
			m_time = time;
			m_group = group;
			m_status = status;
			m_read = read;
			m_delete = false;
		}

		void setDelete(bool del) { m_delete = del; }
		bool isDelete() const { return m_delete; }

	public:
		QString                  m_fromId;
		QString                  m_fromName;
		QString                  m_toId;
		QString                  m_toName;
		QString                  m_sId;
		AddFriendManager::Action m_action;
		QString                  m_message;
		QString                  m_time;
		QString                  m_group;
		int                      m_status;
		int                      m_read;
		bool                     m_delete;
	};

public:
	AddFriendManager(QObject *parent = 0);
	~AddFriendManager();

	bool requestAddFriendList();
	bool addFriendAction(AddFriendManager::Action action, const QString &fromId, const QString &toId, 
		const QString &sId, const QString &message, const QString &group, const QString &group1 = QString());
	bool addFriendConfirm(const QString &sId);
	bool addFriendRead(const QString &sId);
	bool addFriendDelete(const QString &sId, const QString &by);

	// move to roster manager
	bool deleteFriend(const QString &selfId, const QString &otherId);
	//

	QList<AddFriendManager::Item> refreshItems() const;

	static AddFriendManager::Action stringToAction(const QString &str);
	static QString actionToString(AddFriendManager::Action action);

signals:
	void addFriendRequestOK(const QString &sId, int action, const QString &group, const QString &group1);
	void addFriendRequestFailed(const QString &sId, const QString &desc);
	
	void refreshOK();
	void refreshFailed(const QString &desc);

	void addFriendConfirmOK(const QString &sId);
	void addFriendConfirmFailed(const QString &sId, const QString &desc);

	void addFriendDeleteOK(const QString &sId);
	void addFriendDeleteFailed(const QString &sId, const QString &desc);

	// move to roster manager
	void deleteFriendOK(const QString &id);
	void deleteFriendFailed(const QString &id, const QString &desc);
	// 

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	HttpPool           *m_httpPool;
	QList<int>          m_refreshIds;
	QMap<int, QString>  m_requestIds;
	QMap<int, QString>  m_confirmIds;
	QMap<int, QString>  m_readIds;
	QMap<int, QString>  m_deleteIds;
	QList<AddFriendManager::Item> m_refreshItems;

	// move to roster manager
	QMap<int, QString> m_deleteFriendIds;
	// 
};

#endif // ADDFRIENDMANAGER_H
