#ifndef _CONTACTDB_H_
#define _CONTACTDB_H_

#include "DBBase.h"

class QSqlDatabase;

namespace DB
{
	class LastContactDB : public DBBase
	{
	public:
		LastContactDB();

		bool open();
		bool createTable(const QSqlDatabase &db, const QString &tableName);
		bool createIndex(const QSqlDatabase &db, const QString &tableName);

		bool clearLastBody();

		static const QString DB_LASTCONTACT_TABLENAME;
	};
}
#endif // _CONTACTDB_H_