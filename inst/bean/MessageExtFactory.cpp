#include "ShakeMessageExt.h"
#include "ShareMessageExt.h"
#include "ChatMessageExt.h"
#include "SessionMessageExt.h"
#include "AtMessageExt.h"
#include "TipMessageExt.h"
#include "InterphoneMessageExt.h"
#include "SecretMessageExt.h"

#include "MessageExt.h"

namespace bean
{
	MessageExt MessageExtFactory::fromXml( const QDomElement &elem )
	{
		if (elem.isNull())
			return MessageExt();

		QString bodyType = elem.attribute("type");
		if (bodyType == "shake")
		{
			return create(MessageExt_Shake);
		}
		else if (bodyType == "share")
		{
			MessageExt ret = create(MessageExt_Share);
			ret.setData("shareurl", elem.attribute("url"));
			return ret;
		}
		else if (bodyType == "chat")
		{
			MessageExt ret = create(MessageExt_Chat);
			return ret;
		}
		else if (bodyType == "session")
		{
			return create(MessageExt_Session);
		}
		else if (bodyType == "at")
		{
			MessageExt ret = create(MessageExt_At);
			QString at = elem.attribute("at");
			ret.setData("at", at);
			QString atId = elem.attribute("atid");
			ret.setData("atid", atId);
			return ret;
		}
		else if (bodyType == "tip")
		{
			MessageExt ret = create(MessageExt_Tip);
			QString level = elem.attribute("level");
			QString action = elem.attribute("action");
			QString param = elem.attribute("param");
			ret.setData("level", level);
			ret.setData("action", action);
			ret.setData("param", param);
			ret.setData(bean::EXT_DATA_HISTORY_NAME, false);     // do not save to history
			ret.setData(bean::EXT_DATA_LASTCONTACT_NAME, false); // do not add to last contact
			ret.setData(bean::EXT_DATA_CHECKFAILED_NAME, false); // do not check failed
			return ret;
		}
		else if (bodyType == "interphone")
		{
			return create(MessageExt_Interphone);
		}
		else if (bodyType == "secret")
		{
			return create(MessageExt_Secret);
		}
	
		return MessageExt();
	}


	bean::MessageExt MessageExtFactory::create(MessageExtType type)
	{
		bean::MessageExt ext = MessageExt(type);
		if (type == bean::MessageExt_Tip)
		{
			ext.setData(bean::EXT_DATA_HISTORY_NAME, false);     // do not save to history
			ext.setData(bean::EXT_DATA_LASTCONTACT_NAME, false); // do not add to last contact
			ext.setData(bean::EXT_DATA_CHECKFAILED_NAME, false); // do not check failed
		}
		return ext;
	}
}