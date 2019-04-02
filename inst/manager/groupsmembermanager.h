#ifndef GROUPSMEMBERMANAGER_H
#define GROUPSMEMBERMANAGER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QThread>
#include <QScopedPointer>
#include <QStringList>
#include <QList>
#include <QMap>

class GroupManager;
class DiscussManager;
namespace DB {
	class GroupsDB;
};

class GroupsMemberManager : public QObject
{
	Q_OBJECT

public:
	GroupsMemberManager(GroupManager *groupManager, DiscussManager *discussManager, QObject *parent = 0);
	~GroupsMemberManager();

	void startFetch();
	void stopFetch();

	void setGroupNewVersions(const QMap<QString, QString> &groupVersions);
	QString groupNewVersion(const QString &groupId) const;
	QMap<QString, QString> groupOldVersions() const;
	
	void setDiscussNewVersions(const QMap<QString, QString> &discussVersions);
	QString discussNewVersion(const QString &discussId) const;
	QMap<QString, QString> discussOldVersions() const;

	void fetchGroupMembers(const QString &groupId);
	void storeGroupMembers(const QString &groupId, const QString &version, const QByteArray &data);

	void fetchDiscussMembers(const QString &discussId);
	void storeDiscussMembers(const QString &discussId, const QString &version, const QByteArray &data);

private Q_SLOTS:
	void doFetchGroupMembers(const QString &groupId);
	void doFetchDiscussMembers(const QString &discussId);
	void doStoreGroupMembers(const QString &groupId, const QString &version, const QByteArray &data);
	void doStoreDiscussMembers(const QString &discussId, const QString &version, const QByteArray &data);

Q_SIGNALS:
	void groupMemberFetched(
		const QString &groupId, 
		const QString &groupDesc, 
		const QString &groupVersion,
		const QStringList &memberIds, 
		const QStringList &memberNames, 
		const QList<int> &memberIndexes,
		const QStringList &memberCardNames);

	void discussMemberFetched(
		const QString &discussId, 
		const QString &discussVersion,
		const QStringList &members,
		const QStringList &memberNames,
		const QStringList &addedIds,
		const QStringList &cardNames);

private:
	DB::GroupsDB *groupsDB();

private:
	volatile bool                m_started;
	QThread                      m_thread;
	QScopedPointer<DB::GroupsDB> m_groupsDB;

	GroupManager                *m_groupManager;
	QMap<QString, QString>       m_groupOldVersions;
	QMap<QString, QString>       m_groupNewVersions;

	DiscussManager              *m_discussManager;
	QMap<QString, QString>       m_discussOldVersions;
	QMap<QString, QString>       m_discussNewVersions;
};

#endif // GROUPSMEMBERMANAGER_H
