#include "MessageExtBase.h"
#include "MessageExt.h"

namespace bean
{
	const char *EXT_DATA_HISTORY_NAME           = "history";
	const char *EXT_DATA_LASTCONTACT_NAME       = "lastcontact";
	const char *EXT_DATA_CHECKFAILED_NAME       = "checkfailed";

	MessageExt::MessageExt()
		: d(new MessageExtBase())
	{
	}

	MessageExt::MessageExt(MessageExtType type)
		: d(new MessageExtBase(type))
	{
	}

	MessageExt::MessageExt(const MessageExt &other)
		: d(other.d)
	{
	}

	MessageExt::~MessageExt()
	{
	}

	void MessageExt::copy()
	{
		d.detach();
	}

	MessageExtType MessageExt::type() const
	{
		return d->type;
	}

	QVariantMap MessageExt::toJson()
	{
		return d->toJson();
	}

	QDomElement MessageExt::toXml(QDomDocument &doc)
	{
		return d->toXml(doc);
	}

	QString MessageExt::toText(bool isSend, const QString &username, const QString &bodyText)
	{
		return d->toText(isSend, username, bodyText);
	}

	MessageExt& MessageExt::operator=(const MessageExt &other)
	{
		if (this != &other)
		{
			d = other.d;
		}
		return *this;
	}

	void MessageExt::setData(const QString &key, const QVariant &val)
	{
		d->data[key] = val;
	}

	QVariant MessageExt::data(const QString &key) const
	{
		return d->data.value(key);
	}

	QVariant MessageExt::data(const QString &key, const QVariant &defVal)
	{
		if (d->data.contains(key))
		{
			return d->data.value(key);
		}

		return defVal;
	}

	bool MessageExt::contains(const QString &key) const
	{
		return d->data.contains(key);
	}

}
