#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QScopedPointer>
#include "login/ILoginManager.h"

class HttpPool;
class QImage;
class GroupResHandle;
class GroupNtfHandle;

class GroupManager : public QObject, public ILoginProcess
{
	Q_OBJECT
	Q_INTERFACES(ILoginProcess);

public:
	GroupManager(QObject *parent = 0);
	~GroupManager();

public:
	void syncGroups();
	void syncGroupMembers(const QString &id);
	void requestLogo(const QString &id);
	QMap<QString, int> logoVersions() const;
	QString logoPath(const QString &id) const;

	void changeCardName(const QString &groupId, const QString &uid, const QString &cardName);

public:
	virtual bool initObject();
	virtual void removeObject();
	
public:
	// ILoginProcess
	virtual QObject *instance();
	virtual QString name() const;
	virtual bool start();

Q_SIGNALS:
	void finished();
	void error(const QString &err);

Q_SIGNALS:
	void getGroupLogoFinished(const QString &gid, int version, const QImage &logo);

	void groupOK();
	void syncGroupMembersOK(const QString &id);
	void syncGroupMembersFailed(const QString &id, const QString &desc);

	void changeCardNameOK(const QString &groupId);
	void changeCardNameFailed(const QString &groupId, const QString &errMsg);

private slots:
	void onSyncGroupsOK(const QStringList &ids, const QStringList &names, const QList<int> &indice, 
		                const QList<int> &logoVersions, const QStringList &annts, const QStringList &versions);
	void onSyncGroupMembersOK(const QString &gid, const QString &desc, const QString &version, const QStringList &memberIds,
		                      const QStringList &memberNames, const QList<int> &indice, const QStringList &cardNames);
	void onSyncGroupMembersFailed(const QString &gid, const QString &desc);
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	void syncNextGroupMembers();

private:
	QScopedPointer<GroupResHandle> m_pResHandle;
	QScopedPointer<GroupNtfHandle> m_pNtfHandle;

	HttpPool           *m_httpPool;
	QMap<int, QString>  m_requestLogoes;
	QMap<QString, int>  m_logoVersions;

	QStringList         m_syncGroupIds;
};

#endif // GROUPMANAGER_H
