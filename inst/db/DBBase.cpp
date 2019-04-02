#include <QtSql>
#include "DBBase.h"

namespace DB
{
	QString DBBase::DB_TYPE = "SQLITECIPHER";
#if defined(NDEBUG)
	QString DBBase::DB_PASSWD = "1a2b3c4d5e";
#else
	QString DBBase::DB_PASSWD = "";
#endif

	DBBase::DBBase()
	{
		// append to dbs
		s_allDBs.append(this);

		m_Tag = QString("%1_DBBase").arg(m_Connname);
	}

	DBBase::~DBBase()
	{
		// remove from dbs
		if (s_allDBs.contains(this))
			s_allDBs.removeAll(this);

		close();
	}

	QString DBBase::filename() const
	{
		return m_Filename;
	}

	QString DBBase::connectName() const
	{
		return m_Connname;
	}

	bool DBBase::isOpen() const
	{
		bool ret = false;
		do 
		{
			if (m_Connname.isEmpty())
				break;

			if (!QSqlDatabase::contains(m_Connname))
				break;

			ret = QSqlDatabase::database(m_Connname).isOpen();
		} while (0);
		return ret;
	}

	void DBBase::close()
	{
		if (m_Connname.isEmpty())
			return;

		if (QSqlDatabase::contains(m_Connname))
		{
			QSqlDatabase::removeDatabase(m_Connname);
		}
	}

	bool DBBase::insert(const QString& table, const QVariantMap& values)
	{
		bool bRet = false;
		do 
		{
			if (values.isEmpty())
				break;

			if (m_Connname.isEmpty())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid() || !db.isOpen())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QStringList keys = values.keys();
			QString sql = QString("insert into [%1] ([%2]) values (:%3)")
				.arg(table)
				.arg(keys.join("], ["))
				.arg(keys.join(", :"));


			QSqlQuery query(db);
			if (!query.prepare(sql))
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			foreach (QString key, keys)
			{
				query.bindValue(QString(":%1").arg(key), values[key]);
			}

			if (!query.exec())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			m_lastInsertId = query.lastInsertId().toULongLong();
			query.finish();

			bRet = true;
		} while (0);

		return bRet;
	}

	bool DBBase::update(const QString& table, const QVariantMap& values, const QString& whereClause, const QStringList& whereArgs)
	{
		bool bRet = false;
		do 
		{
			if (m_Connname.isEmpty())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid() || !db.isOpen())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QStringList keys = values.keys();
			QString sql;
			if (!whereClause.isEmpty())
			{
				sql = QString("update [%1] set [%2]=? where %3")
					.arg(table)
					.arg(keys.join("]=?, ["))
					.arg(whereClause);
			}
			else
			{
				sql = QString("update [%1] set [%2]=?")
					.arg(table)
					.arg(keys.join("]=?, ["));
			}

			QSqlQuery query(db);
			if (!query.prepare(sql))
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			foreach (QString key, keys)
			{
				query.addBindValue(values.value(key));
			}

			foreach (QString val, whereArgs)
			{
				query.addBindValue(val);
			}

			if (!query.exec())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			bRet = true;
		} while (0);
		return bRet;
	}

	bool DBBase::replace(const QString& table, const QVariantMap& values)
	{
		bool bRet = false;
		do 
		{
			if (m_Connname.isEmpty())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid() || !db.isOpen())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QStringList keys = values.keys();
			QString sql = QString("replace into [%1] ([%2]) values (:%3)")
				.arg(table)
				.arg(keys.join("], ["))
				.arg(keys.join(", :"));


			QSqlQuery query(db);
			if (!query.prepare(sql))
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			// qDebug() << Q_FUNC_INFO << query.executedQuery();

			foreach (QString key, keys)
			{
				query.bindValue(QString(":%1").arg(key), values[key]);
			}

			if (!query.exec())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			bRet = true;
		} while (0);
		return bRet;
	}

	bool DBBase::deleteRows(const QString& table, const QString& whereClause, const QStringList& whereArgs)
	{
		bool bRet = false;
		do 
		{
			if (m_Connname.isEmpty())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid() || !db.isOpen())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QString sql = QString("delete from [%1] ").arg(table);
			
			if (!whereClause.isEmpty())
			{
				sql += QString(" where %1 ").arg(whereClause);
			}

			QSqlQuery query(db);
			if (!query.prepare(sql))
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			if (!whereClause.isEmpty())
			{
				foreach (QString arg, whereArgs)
				{
					query.addBindValue(arg);
				}
			}

			if (!query.exec())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			bRet = true;
		} while (0);
		return bRet;
	}

	QList<QVariantList> DBBase::query(const QString& sql, const QStringList& selectionArgs /* = QStringList */)
	{
		QList<QVariantList> ret;
		do 
		{
			if (m_Connname.isEmpty())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid() || !db.isOpen())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlQuery query(db);
			if (!query.prepare(sql))
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			foreach (QString arg, selectionArgs)
			{
				query.addBindValue(arg);
			}

			if (!query.exec())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << query.lastError();
				break;
			}

			int nCount = query.record().count();
			while (query.next())
			{
				QVariantList vl;
				for (int i = 0; i < nCount; ++i)
				{
					vl << query.value(i);
				}
				ret << vl;
			}
		} while (0);

		return ret;
	}

	QStringList DBBase::fields(const QString& tablename) const
	{
		QStringList ret;
		do 
		{
			if (m_Connname.isEmpty())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlDatabase db = QSqlDatabase::database(m_Connname);
			if (!db.isValid() || !db.isOpen())
			{
				qWarning() << Q_FUNC_INFO << m_Tag << "database has not initialized";
				break;
			}

			QSqlRecord record = db.record(tablename);
			for (int i = 0; i < record.count(); ++i)
			{
				ret << record.fieldName(i);
			}
		} while (0);

		return ret;
	}

	quint64 DBBase::lastInsertId() const
	{
		return m_lastInsertId;
	}

	QList<DBBase *> DBBase::s_allDBs;

	void DBBase::closeAllDBs()
	{
		foreach (DBBase *db, s_allDBs)
		{
			if (db)
			{
				db->close();
			}
		}
	}
}