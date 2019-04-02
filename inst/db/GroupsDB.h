#ifndef _GROUPSDB_H_
#define _GROUPSDB_H_

#include "DBBase.h"
#include <QMap>
#include <QString>
#include <QByteArray>

namespace DB
{
	class GroupsDB : public DBBase
	{
	public:
		GroupsDB(const QString& connSuffix = "");
		bool open();

		QMap<QString, QString> allGroupVersions();
		QMap<QString, QString> allDiscussVersions();
		QByteArray groupMember(const QString &gid);
		QByteArray discussMember(const QString &discussId);
		bool storeGroupMember(const QString &gid, const QString &version, const QByteArray &memberData);
		bool storeDiscussMember(const QString &discussId, const QString &version, const QByteArray &memberData);

	public:
		static const QString DB_GROUPSDB_TABLENAME;
	};
}
#endif // _USERDB_H_