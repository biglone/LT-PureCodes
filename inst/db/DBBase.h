#ifndef _DBBASE_H_
#define _DBBASE_H_
#include <QVariant>
#include <QStringList>
#include <QList>

namespace DB
{
	class DBBase
	{
	public:
		DBBase();
		virtual ~DBBase();

	public:
		virtual QString filename() const;
		virtual QString connectName() const;

		quint64 lastInsertId() const;

		virtual bool open() = 0;
		virtual bool isOpen() const;
		virtual void close();

		virtual bool insert(const QString& table, const QVariantMap& values);
		virtual bool update(const QString& table, const QVariantMap& values, const QString& whereClause, const QStringList& whereArgs);
		virtual bool replace(const QString& table, const QVariantMap& values);
		virtual bool deleteRows(const QString& table, const QString& whereClause, const QStringList& whereArgs);
		virtual QList<QVariantList> query(const QString& sql, const QStringList& selectionArgs = QStringList());

		virtual QStringList fields(const QString& tablename) const;

		static void closeAllDBs();

	public:
		static QString DB_TYPE;
		static QString DB_PASSWD;

	protected:
		QString       m_Filename;
		QString       m_Connname;
		QString       m_Tag;
		quint64       m_lastInsertId;

		static QList<DBBase *> s_allDBs; 
	};
}

#endif //_DBBASE_H_