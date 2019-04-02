#ifndef _MESSAGEVIEW_H_
#define _MESSAGEVIEW_H_

#include <QList>
#include <QVariantMap>
#include "DBBase.h"
#include "bean/bean.h"
#include "MessageBody.h"

namespace DB
{
	class MessageView : public DBBase
	{
	public:
		explicit MessageView(const QString &connSuffix);

		bool open();

	public:
		int getMessageCount(const QString &begindate, const QString &enddate, const QString &keyword);
		QList<bean::MessageBody> getMessages(int nOffset, int nLimit, 
			const QString &begindate, const QString &enddate, const QString &keyword);

	public:
		static const QString DB_MESSAGE_VIEWNAME;
		QString m_selfId;
	};
}

#endif // _MESSAGEVIEW_H_