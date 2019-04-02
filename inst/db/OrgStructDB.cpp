#include <QThread>
#include <QThread>
#include <QtSql>

#include "Account.h"
#include "OrgStructDB.h"

static const char DB_SQL_OS_CREATE[]       = "create table [%1]([id] primary key not null, [uid] not null, [pid] not null, [type] not null, [name] not null, [index] default 1 not null, [timestamp] varchar(64), [userstate] default 1 not null, [frozen] default 0 not null, [leafdept] default 0 not null);";
static const char DB_ORGSTRUCT_DBNAME[]    = "orgdeparts.db";
static const char DB_SQL_OS_ADD_LEAFDEPT[]  = "alter table %1 add leafdept default 0;";

namespace DB
{
	const QString OrgStructDB::DB_ORG_OS_TABLE   = "orgstruct";

	OrgStructDB::OrgStructDB( const QString& connSuffix /*= ""*/ )
	{
		m_Connname = QString("OrgStructDB_%1_%2_%3").arg((int)(QThread::currentThreadId())).arg(DB_ORGSTRUCT_DBNAME).arg(connSuffix);
		m_Tag = QString("[OrgStructDB] %1").arg(m_Connname);
	}

	bool OrgStructDB::open()
	{
		bool ret = false;
		do 
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(DBBase::DB_TYPE, m_Connname);
			if (!db.isValid())
			{
				break;
			}

			Account* pAccount = Account::instance();
			Q_ASSERT(pAccount != NULL);

			m_Filename = pAccount->homeDir().filePath(DB_ORGSTRUCT_DBNAME);
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			QSqlQuery query(db);

			// create os table
			query.clear();
			if (!db.tables().contains(DB_ORG_OS_TABLE))
			{
				QString sql = QString(DB_SQL_OS_CREATE).arg(DB_ORG_OS_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();
			}

			if (!fields(DB_ORG_OS_TABLE).contains("leafdept"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_OS_ADD_LEAFDEPT).arg(DB_ORG_OS_TABLE);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();
			}

			ret = true;
		} while (0);

		return ret;
	}

	bool OrgStructDB::clearOsItems()
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		return deleteRows(DB_ORG_OS_TABLE, "", QStringList());
	}

	bool OrgStructDB::setOsItems(const QVariantList &items)
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		QSqlDatabase db = QSqlDatabase::database(m_Connname);
		if (!db.isValid())
		{
			qWarning() << m_Tag << " db is invalid.";
			return false;
		}

		db.transaction();

		bool bOk = false;
		do 
		{
			bool itemOk = true;
			for (int i = 0; i < items.count(); i++)
			{
				QVariantMap vmap = items[i].toMap();
				if (!replace(DB_ORG_OS_TABLE, vmap))
				{
					qDebug() << Q_FUNC_INFO << " replace " << DB_ORG_OS_TABLE << vmap << " failed.";
					itemOk = false;
					break;
				}
			}

			if (itemOk)
				bOk = true;
		} while (0);

		if (bOk)
		{
			db.commit();
		}
		else
		{
			db.rollback();
		}

		return bOk;
	}

	QVariantList OrgStructDB::osItems(const QString &pid)
	{
		QVariantList result;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return result;
			}
		}

		QString sql = QString("select * from %1 where pid = ?").arg(DB_ORG_OS_TABLE);
		QList<QVariantList> items = query(sql, QStringList() << pid);
		int fieldsCount = fields(DB_ORG_OS_TABLE).count();
		for (int i = 0; i < items.count(); i++)
		{
			QVariantList item = items[i];
			if (item.count() != fieldsCount)
			{
				continue;
			}

			QVariantMap data;
			data["id"] = item[0];
			data["uid"] = item[1];
			data["pid"] = item[2];
			data["type"] = item[3];
			data["name"] = item[4];
			data["index"] = item[5];
			data["timestamp"] = item[6];
			data["userstate"] = item[7];
			data["frozen"] = item[8];
			data["leafdept"] = item[9];
			result.append(data);
		}

		return result;
	}

	QVariantList OrgStructDB::osDeptItems()
	{
		QVariantList result;

		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return result;
			}
		}

		QString sql = QString("select * from %1 where type = ?").arg(DB_ORG_OS_TABLE);
		QList<QVariantList> items = query(sql, QStringList() << "d");
		int fieldsCount = fields(DB_ORG_OS_TABLE).count();
		for (int i = 0; i < items.count(); i++)
		{
			QVariantList item = items[i];
			if (item.count() != fieldsCount)
			{
				continue;
			}

			QVariantMap data;
			data["id"] = item[0];
			data["uid"] = item[1];
			data["pid"] = item[2];
			data["type"] = item[3];
			data["name"] = item[4];
			data["index"] = item[5];
			data["timestamp"] = item[6];
			data["leafdept"] = item[9];
			result.append(data);
		}

		return result;
	}

	bool OrgStructDB::setOsDepyLeafDept(const QString &deptId, bool leafDept)
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		QVariantMap vm;
		vm["leafdept"] = leafDept ? 1 : 0;
		return update(DB_ORG_OS_TABLE, vm, "type='d' and id=?", QStringList() << deptId);
	}

	bool OrgStructDB::clearOsDeptChildren(const QString &deptId)
	{
		if (!isOpen())
		{
			if (!open())
			{
				qWarning() << m_Tag << " open failed.";
				return false;
			}
		}

		return deleteRows(DB_ORG_OS_TABLE, "pid=?", QStringList() << deptId);
	}

}
