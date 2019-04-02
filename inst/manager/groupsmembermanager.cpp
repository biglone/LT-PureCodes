#include "groupsmembermanager.h"
#include "groupmanager.h"
#include "DiscussManager.h"
#include "groupmodeldef.h"
#include "DiscussModeldef.h"
#include "db/GroupsDB.h"

GroupsMemberManager::GroupsMemberManager(GroupManager *groupManager, DiscussManager *discussManager, QObject *parent /*= 0*/)
	: QObject(parent), m_groupManager(groupManager), m_discussManager(discussManager), m_started(false)
{
	Q_ASSERT(m_groupManager);
	Q_ASSERT(m_discussManager);

	moveToThread(&m_thread);
	bool connected = false;
	connected = connect(this, 
		SIGNAL(groupMemberFetched(QString, QString, QString, QStringList, QStringList, QList<int>, QStringList)),
		m_groupManager, 
		SLOT(onSyncGroupMembersOK(QString, QString, QString, QStringList, QStringList, QList<int>, QStringList)));
	Q_ASSERT(connected);

	connected = connect(this,
		SIGNAL(discussMemberFetched(QString, QString, QStringList, QStringList, QStringList, QStringList)),
		m_discussManager,
		SLOT(processDiscussMembers(QString, QString, QStringList, QStringList, QStringList, QStringList)));
	Q_ASSERT(connected);
}

GroupsMemberManager::~GroupsMemberManager()
{
	stopFetch();
}

void GroupsMemberManager::startFetch()
{
	if (m_started)
		stopFetch();

	if (!m_thread.isRunning())
		m_thread.start();

	DB::GroupsDB *groupsDB = this->groupsDB();
	m_groupOldVersions = groupsDB->allGroupVersions();
	m_discussOldVersions = groupsDB->allDiscussVersions();

	m_started = true;
}

void GroupsMemberManager::stopFetch()
{
	m_started = false;

	if (m_thread.isRunning())
	{
		m_thread.quit();
		m_thread.wait();
	}
	m_groupsDB.reset(0);
}

void GroupsMemberManager::setGroupNewVersions(const QMap<QString, QString> &groupVersions)
{
	m_groupNewVersions = groupVersions;
}

QString GroupsMemberManager::groupNewVersion(const QString &groupId) const
{
	return m_groupNewVersions.value(groupId, "");
}

QMap<QString, QString> GroupsMemberManager::groupOldVersions() const
{
	return m_groupOldVersions;
}

void GroupsMemberManager::setDiscussNewVersions(const QMap<QString, QString> &discussVersions)
{
	m_discussNewVersions = discussVersions;
}

QString GroupsMemberManager::discussNewVersion(const QString &discussId) const
{
	return m_discussNewVersions.value(discussId, "");
}

QMap<QString, QString> GroupsMemberManager::discussOldVersions() const
{
	return m_discussOldVersions;
}

void GroupsMemberManager::fetchGroupMembers(const QString &groupId)
{
	if (!m_started)
		return;

	QMetaObject::invokeMethod(this, "doFetchGroupMembers", Qt::QueuedConnection, Q_ARG(QString, groupId));
}

void GroupsMemberManager::storeGroupMembers(const QString &groupId, const QString &version, const QByteArray &data)
{
	if (!m_started)
		return;

	QMetaObject::invokeMethod(this, "doStoreGroupMembers", Qt::QueuedConnection, Q_ARG(QString, groupId),
		Q_ARG(QString, version), Q_ARG(QByteArray, data));
}

void GroupsMemberManager::fetchDiscussMembers(const QString &discussId)
{
	if (!m_started)
		return;

	QMetaObject::invokeMethod(this, "doFetchDiscussMembers", Qt::QueuedConnection, Q_ARG(QString, discussId));
}

void GroupsMemberManager::storeDiscussMembers(const QString &discussId, const QString &version, const QByteArray &data)
{
	if (!m_started)
		return;

	QMetaObject::invokeMethod(this, "doStoreDiscussMembers", Qt::QueuedConnection, Q_ARG(QString, discussId),
		Q_ARG(QString, version), Q_ARG(QByteArray, data));
}

void GroupsMemberManager::doFetchGroupMembers(const QString &groupId)
{
	QString oldVersion = m_groupOldVersions[groupId];
	QString newVersion = m_groupNewVersions[groupId];
	DB::GroupsDB *groupsDB = this->groupsDB();
	if (!oldVersion.isEmpty() && !newVersion.isEmpty() && oldVersion == newVersion)
	{
		// load members
		QByteArray rawData = groupsDB->groupMember(groupId);
		QString desc;
		QStringList memberIds;
		QStringList memberNames;
		QList<int> indice;
		QStringList cardNames;
		if (!GroupModel::parseGroupMemberData(rawData, desc, memberIds, memberNames, indice, cardNames))
		{
			m_groupManager->syncGroupMembers(groupId);
		}
		else
		{
			emit groupMemberFetched(groupId, desc, newVersion, memberIds, memberNames, indice, cardNames);
		}
	}
	else
	{
		m_groupManager->syncGroupMembers(groupId);
	}
}

void GroupsMemberManager::doFetchDiscussMembers(const QString &discussId)
{
	QString oldVersion = m_discussOldVersions[discussId];
	QString newVersion = m_discussNewVersions[discussId];
	DB::GroupsDB *groupsDB = this->groupsDB();
	if (!oldVersion.isEmpty() && !newVersion.isEmpty() && oldVersion == newVersion)
	{
		// load members
		QByteArray rawData = groupsDB->discussMember(discussId);
		QStringList memberIds;
		QStringList memberNames;
		QStringList addedIds;
		QStringList cardNames;
		if (!DiscussModel::parseDiscussMemberData(rawData, memberIds, memberNames, addedIds, cardNames))
		{
			m_discussManager->syncDiscuss(discussId);
		}
		else
		{
			emit discussMemberFetched(discussId, newVersion, memberIds, memberNames, addedIds, cardNames);
		}
	}
	else
	{
		m_discussManager->syncDiscuss(discussId);
	}
}

void GroupsMemberManager::doStoreGroupMembers(const QString &groupId, const QString &version, const QByteArray &data)
{
	DB::GroupsDB *groupsDB = this->groupsDB();
	groupsDB->storeGroupMember(groupId, version, data);

	m_groupOldVersions.insert(groupId, version);
}

void GroupsMemberManager::doStoreDiscussMembers(const QString &discussId, const QString &version, const QByteArray &data)
{
	DB::GroupsDB *groupsDB = this->groupsDB();
	groupsDB->storeDiscussMember(discussId, version, data);

	m_discussOldVersions.insert(discussId, version);
}

DB::GroupsDB *GroupsMemberManager::groupsDB()
{
	if (m_groupsDB.isNull())
	{
		m_groupsDB.reset(new DB::GroupsDB("groupsmembermanager"));
	}
	return m_groupsDB.data();
}