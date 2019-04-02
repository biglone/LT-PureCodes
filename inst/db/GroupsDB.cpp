#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "settings/GlobalSettings.h"
#include "GroupsDB.h"
#include "Account.h"

static const char *kszGroupType   = "group";
static const char *kszDiscussType = "discuss";

const char DB_SQL_GROUPS_CREATE[] = "create table [%1]([id] integer primary key autoincrement, [gid] varchar(64), [gtype] varchar(64), [version] varchar(64), [data] blob);";
const char DB_SQL_GROUPS_CREATE_INDEX[]  = "create unique index [IK_GROUPSIDTYPE] on [%1] ( [gid], [gtype] ); ";

namespace DB
{
	const QString GroupsDB::DB_GROUPSDB_TABLENAME = "groups";

	GroupsDB::GroupsDB(const QString& connSuffix /*= ""*/) : DBBase()
	{	
		m_Connname = QString("GroupsDB_%1_%2").arg(DB_GROUPSDB_TABLENAME).arg(connSuffix);
		m_Tag = QString("[GroupsDB] %1").arg(m_Connname);
	}

	bool GroupsDB::open()
	{
		bool bRet = false;
		do 
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(DBBase::DB_TYPE, m_Connname);
			if (!db.isValid())
			{
				qWarning("%s %s %s no driver.", Q_FUNC_INFO, qPrintable(m_Tag), DBBase::DB_TYPE);
				break;
			}

			Account* pAccount = Account::instance();
			Q_ASSERT(pAccount != NULL);

			m_Filename = pAccount->userDbFilePath();
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			if (!db.tables().contains(DB_GROUPSDB_TABLENAME))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_GROUPS_CREATE).arg(DB_GROUPSDB_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}

				query.clear();

				sql = QString(DB_SQL_GROUPS_CREATE_INDEX).arg(DB_GROUPSDB_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			bRet = true;
		} while (0);
		return bRet;
	}

	QMap<QString, QString> GroupsDB::allGroupVersions()
	{
		QMap<QString, QString> groupVersions;
		if (!isOpen() && !open())
		{
			return groupVersions;
		}

		QString sql = QString("select [gid], [version] from [%1] where gtype = '%2'")
			.arg(DB_GROUPSDB_TABLENAME).arg(kszGroupType);
		QList<QVariantList> data = query(sql);
		foreach (QVariantList vl, data)
		{
			if (vl.isEmpty())
				continue;

			QString gid = vl[0].toString();
			QString ver = vl[1].toString();
			groupVersions.insert(gid, ver);
		}
		return groupVersions;
	}

	QMap<QString, QString> GroupsDB::allDiscussVersions()
	{
		QMap<QString, QString> discussVersions;
		if (!isOpen() && !open())
		{
			return discussVersions;
		}

		QString sql = QString("select [gid], [version] from [%1] where gtype = '%2'")
			.arg(DB_GROUPSDB_TABLENAME).arg(kszDiscussType);
		QList<QVariantList> data = query(sql);
		foreach (QVariantList vl, data)
		{
			if (vl.isEmpty())
				continue;

			QString gid = vl[0].toString();
			QString ver = vl[1].toString();
			discussVersions.insert(gid, ver);
		}
		return discussVersions;
	}

	QByteArray GroupsDB::groupMember(const QString &gid)
	{
		QByteArray memberData;
		if (!isOpen() && !open())
		{
			return memberData;
		}

		QString sql = QString("select [data] from [%1] where gid = '%2' and gtype = '%3'")
			.arg(DB_GROUPSDB_TABLENAME).arg(gid).arg(kszGroupType);
		QList<QVariantList> data = query(sql);
		if (data.count() != 1)
			return memberData;

		QVariantList vl = data[0];
		memberData = vl[0].toByteArray();
		return memberData;
	}

	QByteArray GroupsDB::discussMember(const QString &discussId)
	{
		QByteArray memberData;
		if (!isOpen() && !open())
		{
			return memberData;
		}

		QString sql = QString("select [data] from [%1] where gid = '%2' and gtype = '%3'")
			.arg(DB_GROUPSDB_TABLENAME).arg(discussId).arg(kszDiscussType);
		QList<QVariantList> data = query(sql);
		if (data.count() != 1)
			return memberData;

		QVariantList vl = data[0];
		memberData = vl[0].toByteArray();
		return memberData;
	}

	bool GroupsDB::storeGroupMember(const QString &gid, const QString &version, const QByteArray &memberData)
	{
		if (!isOpen() && !open())
		{
			return false;
		}

		QVariantMap data;
		data.insert("gid", gid);
		data.insert("gtype", kszGroupType);
		data.insert("version", version);
		data.insert("data", memberData);
		return replace(DB_GROUPSDB_TABLENAME, data);
	}

	bool GroupsDB::storeDiscussMember(const QString &discussId, const QString &version, const QByteArray &memberData)
	{
		if (!isOpen() && !open())
		{
			return false;
		}

		QVariantMap data;
		data.insert("gid", discussId);
		data.insert("gtype", kszDiscussType);
		data.insert("version", version);
		data.insert("data", memberData);
		return replace(DB_GROUPSDB_TABLENAME, data);
	}
}