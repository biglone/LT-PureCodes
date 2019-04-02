#ifndef _IOSPUSHNOTIFCATION_H_
#define _IOSPUSHNOTIFCATION_H_

#include "protocol/ProtocolType.h"
#include "net/XmlMsg.h"
#include "net/SeqGenerator.h"

#include "protocol_global.h"

namespace protocol
{
    class PROTOCOL_EXPORT IOSPushNotification
    {
    public:
        struct Param
        {
            std::string                          type;
            std::string                          silence;
            std::string                          from;
            std::string                          to;

            Param() {}

            Param(const Param& other)
            {
                type = other.type;
                silence = other.silence;
                from = other.from;
                to = other.to;
            }

            bool isValid()
            {
                if (type.empty() || from.empty() || to.empty() || silence.empty())
                {
                    return false;
                }

                return true;
            }
        };

        class PROTOCOL_EXPORT Out : public net::XmlMsg
        {
        public:
			Out();

            std::string getBuffer();

			std::string getSeq() { return m_sSeq; }

        public:
            Param param;

		private:
			static net::SeqGenerator s_seqGenerator;

			std::string m_sSeq;
        };	
    };
}
#endif // _IOSPUSHNOTIFCATION_H_
