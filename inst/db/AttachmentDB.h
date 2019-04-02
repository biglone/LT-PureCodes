#ifndef _ATTACHMENTDB_H_
#define _ATTACHMENTDB_H_

#include <QList>
#include <QVariantMap>
#include "DBBase.h"
#include "bean/bean.h"

namespace DB
{
	class AttachmentDB : public DBBase
	{
	public:
		explicit AttachmentDB(const QString &connSuffix);

		bool open();

	public:
		QVariantList getAttachments(int offset = 0, int limit = 60, const QString &beginDate = "", const QString &keyword = "");
		int getAttachmentCount(const QString &beginDate = "", const QString &keyword = "");
		bool remove(int index, bean::MessageType msgType);

		static const QString DB_ATTACH_VIEWNAME; 
		static const QString DB_ATTACH_VIEWNAME2;
		static const QString DB_ATTACH_VIEWNAME3;
	};
}

#endif // _ATTACHMENTDB_H_