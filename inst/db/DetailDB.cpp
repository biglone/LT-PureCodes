#include <QDebug>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>

#include "login/Account.h"
#include "DetailDB.h"
#include "bean/DetailItem.h"

static const char DB_SQL_DETAIL_CREATE[] = "create table [%1]([id] integer primary key autoincrement, "
                                           "[uid] varchar(10), [name] varchar(64),"
										   "[message] varchar(1024), [sex] integer, [birthday] varchar(20), "
										   "[phone1] varchar(20), [phone2] varchar(20), [phone3] varchar(20), [email] varchar(64), "
										   "[depart] varchar(256), [organization] varchar(256), [duty] varchar(100), [area] varchar(64), [jobdesc] varchar(1024), "
										   "[disabled] integer default 0,"
										   "[version] integer default 0, [updatetime]);";
static const char DB_SQL_DETAIL_CREATE_INDEX[]       = "create unique index [IK_DETAILS] on [%1] ( [uid] );";
static const char DB_SQL_DETAIL_ADD_DISABLED[]       = "alter table [%1] add disabled default 0";
static const char DB_DETAIL_DBNAME[]                 = "details.db";

static const char DB_DETAIL_OLD_INDEX[]              = "IK_DETAIL";
static const char DB_DETAIL_INDEX[]                  = "IK_DETAILS";

namespace DB
{
	const QString DetailDB::DB_DETAIL_TABLENAME = "details";
	const QString DetailDB::DB_DETAIL_OLD_TABLENAME = "detail";

	DetailDB::DetailDB(const QString& connSuffix /* = "" */) : DBBase()
	{
		m_Connname = QString("DetailDB_%1_%2_%3").arg((int)(QThread::currentThreadId())).arg(DB_DETAIL_TABLENAME).arg(connSuffix);
		m_Tag = QString("[DetailDB] %1").arg(m_Connname);
	}

	bool DetailDB::open()
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

			m_Filename = pAccount->homeDir().filePath(DB_DETAIL_DBNAME);
			db.setDatabaseName(m_Filename);
			db.setPassword(DBBase::DB_PASSWD);
			if (!db.open())
			{
				qWarning() << m_Tag << db.lastError();
				break;
			}

			// 检查索引有没有创建
			QSqlQuery query(db);
			if (!query.exec("select * from sqlite_master where type='index'"))
			{
				qWarning() << m_Tag << query.lastError();
				break;
			}

			bool bHasOldIndex = false;
			bool bHasNewIndex = false;
			while (query.next())
			{
				QString indexName = query.value(1).toString().toUpper();
				if (indexName == QString(DB_DETAIL_OLD_INDEX))
				{
					bHasOldIndex = true;
				}
				else if (indexName == QString(DB_DETAIL_INDEX))
				{
					bHasNewIndex = true;
				}
			}

			// drop old index
			query.clear();
			if (bHasOldIndex)
			{
				if (!query.exec(QString("drop index [%1]").arg(DB_DETAIL_OLD_INDEX)))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// drop old table
			query.clear();
			if (db.tables().contains(DB_DETAIL_OLD_TABLENAME))
			{
				if (!query.exec(QString("drop table [%1]").arg(DB_DETAIL_OLD_TABLENAME)))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// drop new table without index
			query.clear();
			if (!bHasNewIndex && db.tables().contains(DB_DETAIL_TABLENAME))
			{
				if (!query.exec(QString("drop table [%1]").arg(DB_DETAIL_TABLENAME)))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			// create table and index
			if (!db.tables().contains(DB_DETAIL_TABLENAME))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_DETAIL_CREATE).arg(DB_DETAIL_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
				query.clear();

				sql = QString(DB_SQL_DETAIL_CREATE_INDEX).arg(DB_DETAIL_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			QStringList fields = this->fields(DB_DETAIL_TABLENAME);
			if (!fields.contains("disabled"))
			{
				QSqlQuery query(db);
				QString sql = QString(DB_SQL_DETAIL_ADD_DISABLED).arg(DB_DETAIL_TABLENAME);
				if (!query.exec(sql))
				{
					qWarning() << m_Tag << query.lastError();
					break;
				}
			}

			ret = true;
		} while (0);
		return ret;
	}

	bool DetailDB::writeDetailItem(const bean::DetailItem *pItem)
	{
		bool bRet = false;

		do 
		{
			if (!this->isOpen())
			{
				if (!this->open())
				{
					break;
				}
			}

			QVariantMap vmap = pItem->toDBMap();
			bRet = this->replace(DB::DetailDB::DB_DETAIL_TABLENAME, vmap);

		} while (0);

		return bRet;
	}

	QMap<QString, bean::DetailItem *> DetailDB::readDetailItems()
	{
		QMap<QString, bean::DetailItem *> ret;
		do 
		{
			if (!this->isOpen())
			{
				if (!this->open())
				{
					break;
				}
			}

			QStringList fields = this->fields(DB::DetailDB::DB_DETAIL_TABLENAME);
			QString sql = QString("select * from %1").arg(DB::DetailDB::DB_DETAIL_TABLENAME);
			QList<QVariantList> lvl = this->query(sql);
			foreach (QVariantList vl, lvl)
			{
				if (vl.isEmpty() || vl.length() != fields.length())
					continue;

				bean::DetailItem *pItem = bean::DetailItemFactory::createItem();
				QVariantMap vmap;
				for (int i = 1; i < vl.count(); ++i)
				{
					vmap.insert(fields[i], vl[i]);
				}

				pItem->fromDBMap(vmap);
				ret.insert(pItem->uid(), pItem);
			}

		} while (0);

		return ret;
	}

}