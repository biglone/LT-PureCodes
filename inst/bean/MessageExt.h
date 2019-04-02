#ifndef _MESSAGEEXT_H_
#define _MESSAGEEXT_H_

#include <QString>
#include <QDomElement>
#include <QVariantMap>
#include <QExplicitlySharedDataPointer>
#include "bean.h"

namespace bean
{
	extern const char *EXT_DATA_HISTORY_NAME;
	extern const char *EXT_DATA_LASTCONTACT_NAME;
	extern const char *EXT_DATA_CHECKFAILED_NAME;

	class MessageExtBase;
	class MessageExt
	{
	public:
		MessageExt();
		MessageExt(MessageExtType type);
		MessageExt(const MessageExt &other);
		virtual ~MessageExt();

		void copy();

		MessageExtType type() const;

		void setData(const QString &key, const QVariant &val);
		QVariant data(const QString &key) const;
		QVariant data(const QString &key, const QVariant &defVal);
		bool contains(const QString &key) const;

		QVariantMap toJson();
		QDomElement toXml(QDomDocument &doc);
		QString toText(bool isSend, const QString &username, const QString &bodyText);

		MessageExt& operator=(const MessageExt &other);

	private:
		QExplicitlySharedDataPointer<MessageExtBase> d;
	};

	class MessageExtFactory
	{
	private:
		MessageExtFactory() {}
		~MessageExtFactory() {}

	public:
		static MessageExt fromXml(const QDomElement &elem);
		static MessageExt create(MessageExtType type);
	};
}

Q_DECLARE_METATYPE(bean::MessageExt)

#endif //_MESSAGEEXT_H_
