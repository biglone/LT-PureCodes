#ifndef _PROTOCOLCONST_H_
#define _PROTOCOLCONST_H_

#include "protocol_global.h"

namespace protocol
{

	// error
	//extern const char          ERROR_TIMEOUT[];	

	// xml
	extern const char          ATTRIBUTE_NAME_TYPE[];
	extern const char          ATTRIBUTE_NAME_SEQ[];
	extern const char          ATTRIBUTE_NAME_MODULE[];
	extern const char          ATTRIBUTE_NAME_FROM[];
	extern const char          ATTRIBUTE_NAME_FROMNAME[];
	extern const char          ATTRIBUTE_NAME_TONAME[];
	extern const char          TAG_MESSAGE[];
	extern const char          TAG_HEARTBEAT[];
	extern const char          ATTRIBUTE_REQUEST[];
	extern const char          ATTRIBUTE_RESPONSE[];
	extern const char          ATTRIBUTE_NOTIFICATION[];

	extern const char          ATTRIBUTE_ERRCODE[];
	extern const char          ATTRIBUTE_ERRMSG[];

	extern const char          TAG_LOGIN[];
	extern const char          ATTRIBUTE_NAME_PASSWD[];
	extern const char          ATTRIBUTE_NAME_RESOURCE[];
	extern const char          ATTRIBUTE_NAME_PLATFORM[];
	extern const char          ATTRIBUTE_NAME_VIOLENT[];
	extern const char          ATTRIBUTE_NAME_BALANCE[];
	extern const char          TAG_PARAMETERS[];
	extern const char          TAG_PARAMETER[];
	extern const char          ATTRIBUTE_NAME_ID[];
	extern const char          ATTRIBUTE_NAME_VALUE[];
	extern const char          TAG_MODULES[];
	extern const char          TAG_MODULE[];
	extern const char          TAG_SERVICES[];
	extern const char          TAG_SERVICE[];
	extern const char          TAG_PSGS[];
	extern const char          TAG_PSG[];
	extern const char          TAG_SGS[];
	extern const char          TAG_SG[];
	extern const char          ATTRIBUTE_NAME_MAIN[];

	extern const char          TAG_RELOGIN[];

	extern const char          TAG_LOGOUT[];

	extern const char          TAG_UPDATE[];
	extern const char          ATTRIBUTE_NAME_TO[];
	extern const char          ATTRIBUTE_NAME_LENGTH[];
	extern const char          ATTRIBUTE_NAME_MESSAGE[];

	extern const char          TAG_UPDATE_PLUGINS[];
	extern const char          TAG_PLUGIN[];
	extern const char          ATTRIBUTE_NAME_VERSION[];

	extern const char          TAG_DOWNLOAD[];
	extern const char          TAG_DATA[];

	extern const char          TAG_TIMESYNC[];

	extern const char          TAG_PASSWD[];

	extern const char          TAG_FEEDBACK[];

	extern const char          TAG_KICK[];
	extern const char          ATTRIBUTE_NAME_CONTENT[];
	extern const char          ATTRIBUTE_BASE[];

	extern const char          TAG_ROSTER[];
	extern const char          TAG_ITEM[];
	extern const char          ATTRIBUTE_NAME_NAME[];
	extern const char          ATTRIBUTE_NAME_GROUP[];
	extern const char          ATTRIBUTE_NAME_INDEX[];
	extern const char          ATTRIBUTE_NAME_DESC[];
	extern const char          ATTRIBUTE_NAME_MODIFY[];
	extern const char          ATTRIBUTE_NAME_SYNC[];
	extern const char          ATTRIBUTE_IM[];

	extern const char          TAG_GROUP[];
	extern const char          TAG_USER[];

	extern const char          TAG_PRESENCE[];
	extern const char          TAG_TTYPE[];
	extern const char          TAG_SHOW[];
	extern const char          TAG_STATUS[];

	extern const char          TAG_EXT[];
	extern const char          TAG_SUBJECT[];
	extern const char          TAG_BODY[];
	extern const char          TAG_ATTACHMENTS[];
	extern const char          TAG_ATTACHMENT[];
	extern const char          TAG_NAME[];
	extern const char          TAG_FORMAT[];
	extern const char          TAG_ID[];
	extern const char          TAG_SIZE[];
	extern const char          ATTRIBUTE_NAME_FONT[];
	extern const char          ATTRIBUTE_NAME_TIME[];
	extern const char          ATTRIBUTE_NAME_SOURCE[];
	extern const char          ATTRIBUTE_NAME_AUTODISPLAY[];
	extern const char          ATTRIBUTE_NAME_AUTODOWNLOAD[];
	extern const char          ATTRIBUTE_NAME_DIR[];
	extern const char          ATTRIBUTE_NAME_URL[];
	extern const char          ATTRIBUTE_NAME_AT[];
	extern const char          ATTRIBUTE_NAME_AT_ID[];
	extern const char          ATTRIBUTE_NAME_CREATOR[];
	extern const char          ATTRIBUTE_NAME_ADDED[];
	extern const char          ATTRIBUTE_NAME_ENCRYPT[];
	extern const char          ATTRIBUTE_NAME_PICWIDTH[];
	extern const char          ATTRIBUTE_NAME_PICHEIGHT[];

	extern const char          ATTRIBUTE_NAME_IP[];
	extern const char          ATTRIBUTE_NAME_PORT[];

	extern const char          TAG_TALKBEGIN[];
	extern const char          ATTRIBUTE_MODULE_TALK[];
	extern const char          ATTRIBUTE_NAME_BEGINSEQ[];
	extern const char          TAG_AUDIODESC[];
	extern const char          ATTRIBUTE_NAME_BIT[];
	extern const char          ATTRIBUTE_NAME_CHAN[];
	extern const char          ATTRIBUTE_NAME_RATE[];
	extern const char          ATTRIBUTE_NAME_FRAME[];
	extern const char          ATTRIBUTE_NAME_BEGINTIME[];
	extern const char          TAG_TOS[];
	extern const char          TAG_TO[];
	extern const char          TAG_TALKEND[];
	extern const char          ATTRIBUTE_NAME_ENDSEQ[];
	extern const char          ATTRIBUTE_NAME_REASON[];
	extern const char          TAG_RECVEND[];
	extern const char          TAG_RESEND[];
	extern const char          ATTRIBUTE_NAME_AUDIOPACKET[];

	extern const char          TAG_START[];
	extern const char          ATTRIBUTE_NAME_REQAUDIO[];
	extern const char          ATTRIBUTE_MODULE_VIDEO[];
	extern const char          TAG_OK[];
	extern const char          ATTRIBUTE_NAME_NEWID[];
	extern const char          ATTRIBUTE_NAME_VIDEOPARAM[];
	extern const char          ATTRIBUTE_NAME_WIDTH[];
	extern const char          ATTRIBUTE_NAME_HEIGHT[];
	extern const char          ATTRIBUTE_NAME_CODEC[];
	extern const char          ATTRIBUTE_NAME_CONFIG[];
	extern const char          ATTRIBUTE_NAME_FPS[];
	
	extern const char          TAG_REJECT[];
	extern const char          TAG_REPORT[];
	extern const char          TAG_STOP[];
	extern const char          TAG_NOTIFY[];

	extern const char          TAG_FIND[];
	extern const char          TAG_PHOTO[];

	extern const char          TAG_MODIFY[];

	extern const char          TAG_VIDEO[];
	extern const char          TAG_AUDIO[];

	extern const char          ATTRIBUTE_MODULE_RTC[];
	extern const char          TAG_INVITE[];
	extern const char          TAG_RINGING[];
	extern const char          TAG_BYE[];
	extern const char          TAG_ACK[];
	extern const char          TAG_REASON[];
	extern const char          TAG_SDP[];
	extern const char          TAG_ICE_CANDIDATE[];
	
	extern const char          ATTRIBUTE_NAME_RECV[];
	extern const char          ATTRIBUTE_NAME_SEND[];
	extern const char          ATTRIBUTE_NAME_ADDR[];

	extern const char          ATTRIBUTE_MSG[];
	extern const char          TAG_MSG[];
	extern const char          ATTRIBUTE_NAME_FTYPE[];
	extern const char          ATTRIBUTE_NAME_COUNT[];
	extern const char          ATTRIBUTE_NAME_TS[];
	extern const char          ATTRIBUTE_NAME_TS2[];
	extern const char          ATTRIBUTE_NAME_TSNOW[];
	extern const char          ATTRIBUTE_NAME_SD[];
	extern const char          ATTRIBUTE_NAME_NUMBER[];
	extern const char          ATTRIBUTE_NAME_TIMESTAMP[];

	extern const char          ATTRIBUTE_ORG[];
	extern const char          TAG_ORG[];
	extern const char          TAG_ORG_STRUCT[];

	//
	extern const char          ATTRIBUTE_MODULE_DISCUSS[];
	extern const char          TAG_DISCUSS[];
	extern const char          ATTRIBUTE_NAME_CREATE[];
	extern const char          ATTRIBUTE_NAME_QUIT[];
	extern const char          ATTRIBUTE_NAME_ADD[];
	extern const char          ATTRIBUTE_NAME_CHANGENAME[];
	extern const char          ATTRIBUTE_NAME_CHANGECARDNAME[];
	
	extern const char          TAG_LOGO_VERSION[];
	extern const char          TAG_LOGO[];
	extern const char          TAG_ANNT[];

	extern const char          ATTRIBUTE_MODULE_CONFIG[];
	extern const char          TAG_CONFIG[];
	extern const char          TAG_CONF1[];
	extern const char          TAG_CONF2[];
	extern const char          TAG_CONF3[];
	extern const char          TAG_CONF4[];
	extern const char          TAG_CONF5[];
	extern const char          TAG_CONF6[];
	extern const char          TAG_CONF7[];
	extern const char          TAG_CONF8[];
	extern const char          TAG_CONF9[];
	extern const char          TAG_GROUPS[];
	extern const char          TAG_IDS[];
	extern const char		   TAG_SILENCE[];

	// notice
	extern const char          TAG_NOTICE[];
	extern const char          ATTRIBUTE_EVENT[];
	extern const char		   EVENT_ID[];

	// search
	extern const char          TAG_SEARCH[];
	extern const char          ATTRIBUTE_NAME_SEX[];
	extern const char          ATTRIBUTE_NAME_DEPART[];
	extern const char          ATTRIBUTE_NAME_PHONE[];
	extern const char          ATTRIBUTE_NAME_MORE[];

	// tip
	extern const char          TAG_TIP[];
	extern const char          ATTRIBUTE_NAME_ACTION[];

	// interphone
	extern const char          ATTRIBUTE_MODULE_INTERPHONE[];
	extern const char          TAG_INTERPHONES[];
	extern const char          TAG_INTERPHONE[];
	extern const char          TAG_MEMBER[];
	extern const char          TAG_SPEAK[];
	extern const char          ATTRIBUTE_NAME_SPEAKER[];
	extern const char          ATTRIBUTE_NAME_ATTACHTYPE[];
	extern const char          ATTRIBUTE_NAME_ATTACHID[];
	extern const char          ATTRIBUTE_NAME_MEMBERCOUNT[];

	extern const char          ATTRIBUTE_NAME_FULLTO[];
	
	extern const char          ATTRIBUTE_ONLINE[];

	extern const char          TAG_WITHDRAW[];
	extern const char          TAG_WITHDRAWS[];
	extern const char          ATTRIBUTE_NAME_UID[];
	extern const char          ATTRIBUTE_NAME_WITHDRAWID[];

	extern const char          ATTRIBUTE_NAME_CARDNAME[];

	extern const char		   ATTRIBUTE_MODLE_IOS_PUSH[];	
	extern const char		   ATTRIBUTE_PUSH_NOTIFICATION[];
	extern const char		   ATTRIBUTE_SILENCE[];
}

#endif