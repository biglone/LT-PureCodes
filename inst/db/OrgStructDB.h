#ifndef _ORGSTRUCTDB_H_
#define _ORGSTRUCTDB_H_

#include "DBBase.h"

namespace DB
{
	class OrgStructDB : public DBBase
	{
	public:
		explicit OrgStructDB(const QString& connSuffix = "");

		bool open();

		bool clearOsItems();
		bool setOsItems(const QVariantList &items);
		QVariantList osItems(const QString &pid);

		QVariantList osDeptItems();

		bool setOsDepyLeafDept(const QString &deptId, bool leafDept);

		bool clearOsDeptChildren(const QString &deptId);

		static const QString DB_ORG_OS_TABLE;
	};
}

#endif //_ORGSTRUCTDB_H_
