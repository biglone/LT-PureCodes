#include "MessageExtBase.h"
#include "AtMessageExt.h"
#include "ChatMessageExt.h"
#include "SessionMessageExt.h"
#include "ShakeMessageExt.h"
#include "ShareMessageExt.h"
#include "TipMessageExt.h"
#include "InterphoneMessageExt.h"
#include "SecretMessageExt.h"

namespace bean
{
	MessageExtBase::MessageExtBase()
		: type(MessageExt_Invalid)
	{
	}

	MessageExtBase::MessageExtBase(const MessageExtBase &other)
	{
		type = other.type;
		data = other.data;
	}

	MessageExtBase::MessageExtBase(bean::MessageExtType type)
	{
		this->type = type;
	}

	QVariantMap MessageExtBase::toJson()
	{
		switch (type)
		{
		case MessageExt_Chat:
			return ChatMessageExt::toJson(data);
		case MessageExt_Shake:
			return ShakeMessageExt::toJson(data);
		case MessageExt_Session:
			return SessionMessageExt::toJson(data);
		case MessageExt_Share:
			return ShareMessageExt::toJson(data);
		case MessageExt_At:
			return AtMessageExt::toJson(data);
		case MessageExt_Tip:
			return TipMessageExt::toJson(data);
		case MessageExt_Interphone:
			return InterphoneMessageExt::toJson(data);
		case MessageExt_Secret:
			return SecretMessageExt::toJson(data);
		default:
			return QVariantMap();
		}
	}

	QDomElement MessageExtBase::toXml(QDomDocument &doc)
	{
		switch (type)
		{
		case MessageExt_Chat:
			return ChatMessageExt::toXml(data, doc);
		case MessageExt_Shake:
			return ShakeMessageExt::toXml(data, doc);
		case MessageExt_Session:
			return SessionMessageExt::toXml(data, doc);
		case MessageExt_Share:
			return ShareMessageExt::toXml(data, doc);
		case MessageExt_At:
			return AtMessageExt::toXml(data, doc);
		case MessageExt_Tip:
			return TipMessageExt::toXml(data, doc);
		case MessageExt_Interphone:
			return InterphoneMessageExt::toXml(data, doc);
		case MessageExt_Secret:
			return SecretMessageExt::toXml(data, doc);
		default:
			return QDomElement();
		}
	}

	QString MessageExtBase::toText(bool isSend, const QString &username, const QString &bodyText)
	{
		switch (type)
		{
		case MessageExt_Chat:
			return ChatMessageExt::toText(data, isSend, username, bodyText);
		case MessageExt_Shake:
			return ShakeMessageExt::toText(data, isSend, username, bodyText);
		case MessageExt_Session:
			return SessionMessageExt::toText(data, isSend, username, bodyText);
		case MessageExt_Share:
			return ShareMessageExt::toText(data, isSend, username, bodyText);
		case MessageExt_At:
			return AtMessageExt::toText(data, isSend, username, bodyText);
		case MessageExt_Tip:
			return TipMessageExt::toText(data, isSend, username, bodyText);
		case MessageExt_Interphone:
			return InterphoneMessageExt::toText(data, isSend, username, bodyText);
		case MessageExt_Secret:
			return SecretMessageExt::toText(data, isSend, username, bodyText);
		default:
			return bodyText;
		}
	}


}