#ifndef _DISCUSSMANAGER_H_
#define _DISCUSSMANAGER_H_

#include <QStringList>
#include <QVariant>
#include <QObject>
#include <QScopedPointer>
#include "login/ILoginManager.h"

class DiscussResHandle;
class DiscussNtfHandle;
class HttpPool;

class DiscussManager : public QObject, public ILoginProcess
{
	Q_OBJECT
	Q_INTERFACES(ILoginProcess);

public:
	enum Type
	{
		Invalid,
		CreateAndAdd,
		Add,
		Quit,
		Sync,
		ChangeName,
		ChangeCardName
	};

public:
	explicit DiscussManager(QObject *parent = 0);
	virtual ~DiscussManager();

public:
	bool initObject();
	void removeObject();

public slots:
	void syncDiscuss(const QString &discussId = QString());

	int createDiscuss(const QString &name, const QStringList &uids);

	int addMembers(const QString &id, const QString &name, const QStringList &uids);

	int quitDiscuss(const QString &id, const QString &name, const QString &uid);

	int changeName(const QString &id, const QString &name);

	bool kick(const QString &discussId, const QString &uid, const QString &by);

	bool disband(const QString &discussId, const QString &by);

	int changeCardName(const QString &discussId, const QString &discussName, const QString &uid, const QString &cardName); 

public:
	// ILoginProcess
	virtual QObject* instance();
	virtual QString name() const;
	virtual bool start();

Q_SIGNALS:
	void finished();
	void error(const QString &err);

Q_SIGNALS:
	void createdDiscuss(int handleId, const QString &id);
	void addedMembers(int handleId, const QString &id);
	void quitedDiscuss(int handleId, const QString &id);
	void quitedDiscuss(const QString &id);
	void nameChanged(int handleId, const QString &id, const QString &name);
	void cardNameChanged(int handleId, const QString &id, const QString &name);
	void discussError(int handleId, int type, const QString &errmsg, const QString &id, const QString &name, const QStringList &members);

	void notifyDiscussChanged(const QString &id);

	void discussOK();

	void discussKickFailed(const QString &discussId, const QString &uid, const QString &errMsg);
	void discussKickOK(const QString &discussId, const QString &uid);
	
	void discussDisbandFailed(const QString &discussId, const QString &errMsg);
	void discussDisbandOK(const QString &discussId);

private slots:
	void processDiscussList(const QStringList &ids, const QStringList &names, const QStringList &creators, 
		const QStringList &times, const QStringList &versions);
	void processDiscussMembers(const QString &id, const QString &name, 
							   const QString &creator, const QString &time, const QString &version,
							   const QStringList &members, const QStringList &memberNames, 
							   const QStringList &addedIds = QStringList(),
							   const QStringList &cardNames = QStringList());
	void processDiscussMembers(const QString &id, const QString &version,
		                       const QStringList &members, const QStringList &memberNames, 
							   const QStringList &addedIds,
							   const QStringList &cardNames);
	void onDiscussMembersFailed(const QString &id, const QString &desc);
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	void syncNextDiscussMembers();

private:
	QScopedPointer<DiscussResHandle>    m_pResHandle; // 讨论组response操作处理对象
	QScopedPointer<DiscussNtfHandle>    m_pNtfHandle; // 讨论组notification下发处理对象

	HttpPool                           *m_pHttpPool;
	QMap<int, QPair<QString, QString>>  m_kickIds;
	QMap<int, QString>                  m_disbandIds;

	QStringList                         m_syncDiscussIds;
};

#endif //_DISCUSSMANAGER_H_
