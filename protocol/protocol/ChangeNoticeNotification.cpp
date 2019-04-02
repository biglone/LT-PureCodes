#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "ChangeNoticeNotification.h"

namespace protocol
{
	ChangeNoticeNotification::ChangeNoticeNotification()
	{
		m_events.clear();
	}

	bool ChangeNoticeNotification::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误

		do
		{
			if (!pnIks)
				break;

			m_events.clear();

			iks *notice = pnIks;
			while (notice)
			{
				char *szNotice = iks_name(notice);
				if (0 == iks_strcmp(szNotice, TAG_NOTICE))
				{
					char *szEventName = iks_find_attrib(notice, ATTRIBUTE_EVENT);
					if (!szEventName)
					{
						continue;
					}

					std::string paramValue;
					iks *undefined = iks_find(notice, "undefined");
					if (undefined)
					{
						char *szUndefined = iks_cdata(iks_child(undefined));
						if (szUndefined)
							paramValue = szUndefined;
					}

					ChangeNoticeNotification::Event event;
					event.name = szEventName;
					event.param.v = paramValue;
					m_events.push_back(event);
				}
				notice = iks_next_tag(notice);
			}
			
			bOk = true;
		} while (false);

		return bOk;
	}

	int ChangeNoticeNotification::getNotificationType()
	{
		return protocol::CHANGENOTICE;
	}

	std::vector<ChangeNoticeNotification::Event> ChangeNoticeNotification::getEvents() const
	{
		return m_events;
	}
}