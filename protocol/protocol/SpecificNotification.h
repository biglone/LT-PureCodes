#ifndef _SPECIFICNOTIFICATION_H_
#define _SPECIFICNOTIFICATION_H_

#include <string>
#include "iks/iksemel.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT SpecificNotification
	{
	public:
		SpecificNotification()
			: m_sModel(""), m_sSpFrom(""), m_sTagName("")
		{}

		virtual ~SpecificNotification() {}

		std::string getModel() const { return m_sModel; }
		std::string getSpFrom() const { return m_sSpFrom; }
		std::string getTagName() const { return m_sTagName; }

		/**
		 * 解析具体的notification信令
		 * @param element
		 * @return 信令不合法返回false;信令合法返回true
		 */
		virtual bool Parse(iks* pnIks) = 0;

		virtual int getNotificationType() = 0;

	protected:
		void setModel(const std::string& rsModel) { m_sModel = rsModel; }
		void setSpFrom(const std::string& rsFrom) { m_sSpFrom = rsFrom; }
		void setTagName(const std::string& rsName) { m_sTagName = rsName; }

	private:
		std::string       m_sModel;
		std::string       m_sSpFrom;
		std::string       m_sTagName;

	private:
		friend class NotificationFactory;
	};
}
#endif
//SpecificNotification_H_