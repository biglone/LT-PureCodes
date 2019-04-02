#ifndef _DETAILDB_H_
#define _DETAILDB_H_

#include "DBBase.h"

namespace bean
{
	class DetailItem;
}

namespace DB
{
	class DetailDB : public DBBase
	{
	public:
		explicit DetailDB(const QString& connSuffix = "");

		bool open();

		bool writeDetailItem(const bean::DetailItem *pItem);
		QMap<QString, bean::DetailItem *> readDetailItems();

		static const QString DB_DETAIL_TABLENAME;
		static const QString DB_DETAIL_OLD_TABLENAME;
	};

}
#endif //_DETAILDB_H_