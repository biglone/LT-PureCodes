#ifndef _HEARTBEATMESSAGE_H_
#define _HEARTBEATMESSAGE_H_

#include "net/XmlMsg.h"
#include "SpecificNotification.h"
#include "net/SeqGenerator.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT HeartbeatMessage
	{
	private:
		HeartbeatMessage();
		~HeartbeatMessage();
	public:
		class PROTOCOL_EXPORT In : public SpecificNotification
		{
		public:
			int getNotificationType();

			bool Parse(iks* pnIks);

			std::string getSeq() const;

		public:
			std::string m_sSeq;
		};

		class PROTOCOL_EXPORT Out: public net::XmlMsg
		{
		public:
			Out();

			std::string getBuffer();

			std::string getSeq() { return m_sSeq; }

		private:
			static net::SeqGenerator s_seqGenerator;

			std::string m_sSeq;
		};
	};
}

#endif
