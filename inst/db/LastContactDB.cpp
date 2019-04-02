#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "login/Account.h"
#include "LastContactDB.h"

const char DB_SQL_LASTCONTACT_CREATE[]             = "create table [%1] ([id] integer primary key not null, [uid], [uname], [type] default 'chat', [lastbody], [lasttime], [hasAttachment] default 0, [members], [timestamp] default '', [senduid] default '')";
const char DB_SQL_LASTCONTACT_CREATE_INDEX[]       = "create unique index [IK_LASTCONTACT] on [%1] ( [uid], [type] ); ";

// for data transfer
const char DB_SQL_LASTCONTACT_INSERT_DATA[]        = "INSERT INTO %1 SELECT id, uid, uname, type, lastbody, lasttime, hasAttachment, null FROM %2;";
const char DB_SQL_LASTCONTACT_DROP_TABLE[]         = "DROP TABLE %1;";
const char DB_SQL_LASTCONTACT_RENAME_TABLE[]       = "ALTER TABLE %1 RENAME TO %2;";
static const char DB_LASTCONTACT_TMP_TABLENAME[]   = "lastcontact_tmp";
const char DB_SQL_LASTCONTACT_ADD_TIMESTAMP[]      = "ALTER TABLE %1 ADD timestamp default '';";
const char DB_SQL_LASTCONTACT_ADD_SENDUID[]        = "ALTER TABLE %1 ADD senduid default '';";

namespace DB
{
	const QString LastContactDB::DB_LASTCONTACT_TABLENAME = "lastcontact";
	LastContactDB::LastContactDB() : DBBase()
	{
		m_Connname = QString("LastContactDB_%1").arg(DB_LASTCONTACT_TABLENAME);
		m_Tag = QString("[LastContactDB] %1").arg(m_Connname);
	}

	bool LastContactDB::open()
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

			m_Filename = pAccount->userDbFilePath();
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			if (!db.tables().contains(DB_LASTCONTACT_TABLENAME))
			{
				if (!createTable(db, DB_LASTCONTACT_TABLENAME))
					break;

				if (!createIndex(db, DB_LASTCONTACT_TABLENAME))
					break;
			}
			else
			{
				QStringList fields = this->fields(DB_LASTCONTACT_TABLENAME);
				if (!fields.contains("members"))
				{
					// transfer data to a new table
					bool ok = false;
					db.transaction();

					do
					{
						// create a tmp table
						if (!createTable(db, DB_LASTCONTACT_TMP_TABLENAME))
							break;

						// move all data from the old table
						QSqlQuery query(db);
						QString sql = QString(DB_SQL_LASTCONTACT_INSERT_DATA).arg(DB_LASTCONTACT_TMP_TABLENAME).arg(DB_LASTCONTACT_TABLENAME);
						if (!query.exec(sql))
						{
							qWarning() << m_Tag << query.lastError();
							break;
						}
						query.clear();

						// drop original table
						sql = QString(DB_SQL_LASTCONTACT_DROP_TABLE).arg(DB_LASTCONTACT_TABLENAME);
						if (!query.exec(sql))
						{
							qWarning() << m_Tag << query.lastError();
							break;
						}
						query.clear();

						// create index on tmp table
						if (!createIndex(db, DB_LASTCONTACT_TMP_TABLENAME))
							break;

						// rename tmp table to original table
						sql = QString(DB_SQL_LASTCONTACT_RENAME_TABLE).arg(DB_LASTCONTACT_TMP_TABLENAME).arg(DB_LASTCONTACT_TABLENAME);
						if (!query.exec(sql))
						{
							qWarning() << m_Tag << query.lastError();
							break;
						}
						query.clear();

						ok = true;
					} while (0);

					if (ok)
					{
						db.commit();
					}

					if (!ok)
					{
						db.rollback();
						break;
					}
				}

				if (!fields.contains("timestamp"))
				{					
					QSqlQuery query(db);
					QString sql = QString(DB_SQL_LASTCONTACT_ADD_TIMESTAMP).arg(DB_LASTCONTACT_TABLENAME);
					if (!query.exec(sql))
					{
						qWarning() << m_Tag << query.lastError();
						break;
					}
				}

				if (!fields.contains("senduid"))
				{
					QSqlQuery query(db);
					QString sql = QString(DB_SQL_LASTCONTACT_ADD_SENDUID).arg(DB_LASTCONTACT_TABLENAME);
					if (!query.exec(sql))
					{
						qWarning() << m_Tag << query.lastError();
						break;
					}
				}
			}

			ret = true;
		} while (0);
		return ret;
	}

	bool LastContactDB::createTable(const QSqlDatabase &db, const QString &tableName)
	{
		bool ok = false;

		do
		{
			QSqlQuery query(db);
			QString sql = QString(DB_SQL_LASTCONTACT_CREATE).arg(tableName);
			if (!query.exec(sql))
			{
				qWarning() << m_Tag << query.lastError();
				break;
			}

			ok = true;
		} while (0);

		return ok;
	}

	bool LastContactDB::createIndex(const QSqlDatabase &db, const QString &tableName)
	{
		bool ok = false;

		do
		{
			QSqlQuery query(db);
			QString sql = QString(DB_SQL_LASTCONTACT_CREATE_INDEX).arg(tableName);
			if (!query.exec(sql))
			{
				qWarning() << m_Tag << query.lastError();
				break;
			}

			ok = true;
		} while (0);

		return ok;
	}

	bool LastContactDB::clearLastBody()
	{
		bool ret = false;
		do 
		{
			if (!isOpen())
			{
				if (!open())
				{
					break;
				}
			}

			QVariantMap values;
			values["lastbody"] = "";
			ret = update(DB_LASTCONTACT_TABLENAME, values, QString(), QStringList());
			
		} while (0);

		return ret;
	}
}