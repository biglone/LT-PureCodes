#include "ProtocolConst.h"

namespace protocol 
{

	//const char          ERROR_TIMEOUT[]                  = "timeout";

	const char          ATTRIBUTE_NAME_TYPE[]            = "type";         
	const char          ATTRIBUTE_NAME_SEQ[]             = "seq";          
	const char          ATTRIBUTE_NAME_MODULE[]          = "module";       
	const char          ATTRIBUTE_NAME_FROM[]            = "from";
	const char          ATTRIBUTE_NAME_FROMNAME[]        = "fromname";
	const char          ATTRIBUTE_NAME_TONAME[]          = "toname";
	const char          TAG_MESSAGE[]                    = "message";      
	const char          TAG_HEARTBEAT[]                  = "heartbeat";    
	const char          ATTRIBUTE_REQUEST[]              = "request";      
	const char          ATTRIBUTE_RESPONSE[]             = "response";     
	const char          ATTRIBUTE_NOTIFICATION[]         = "notification"; 

	const char          ATTRIBUTE_ERRCODE[]              = "errcode";      
	const char          ATTRIBUTE_ERRMSG[]               = "errmsg";       

	const char          TAG_LOGIN[]                      = "login";   
	const char          ATTRIBUTE_NAME_PASSWD[]          = "passwd";
	const char          ATTRIBUTE_NAME_RESOURCE[]        = "resource";
	const char          ATTRIBUTE_NAME_PLATFORM[]        = "platform";
	const char          ATTRIBUTE_NAME_VIOLENT[]         = "violent";
	const char          ATTRIBUTE_NAME_BALANCE[]         = "balance";
	const char          TAG_PARAMETERS[]                 = "parameters";   
	const char          TAG_PARAMETER[]                  = "parameter";    
	const char          ATTRIBUTE_NAME_ID[]              = "id";           
	const char          ATTRIBUTE_NAME_VALUE[]           = "value";        
	const char          TAG_MODULES[]                    = "modules";      
	const char          TAG_MODULE[]                     = "module";       
	const char          TAG_SERVICES[]                   = "services";     
	const char          TAG_SERVICE[]                    = "service";    
	const char          TAG_PSGS[]                       = "psgs";
	const char          TAG_PSG[]                        = "psg";
	const char          TAG_SGS[]                        = "sgs";
	const char          TAG_SG[]                         = "sg";
	const char          ATTRIBUTE_NAME_MAIN[]            = "main";

	const char          TAG_RELOGIN[]                    = "relogin";

	const char          TAG_LOGOUT[]                     = "logout";       

	const char          TAG_UPDATE[]                     = "update";       
	const char          ATTRIBUTE_NAME_TO[]              = "to";           
	const char          ATTRIBUTE_NAME_LENGTH[]          = "length";       
	const char          ATTRIBUTE_NAME_MESSAGE[]         = "message";      

	const char          TAG_UPDATE_PLUGINS[]             = "update-plugins";
	const char          TAG_PLUGIN[]                     = "plugin";       
	const char          ATTRIBUTE_NAME_VERSION[]         = "version";      

	const char          TAG_DOWNLOAD[]                   = "download";     
	const char          TAG_DATA[]                       = "data";         

	const char          TAG_TIMESYNC[]                   = "timesync";     

	const char          TAG_PASSWD[]                     = "passwd";       

	const char          TAG_FEEDBACK[]                   = "feedback";     

	const char          TAG_KICK[]                       = "kick";         
	const char          ATTRIBUTE_NAME_CONTENT[]         = "content";      
	const char          ATTRIBUTE_BASE[]                 = "base";         

	const char          TAG_ROSTER[]                     = "roster";       
	const char          TAG_ITEM[]                       = "item";         
	const char          ATTRIBUTE_NAME_NAME[]            = "name";         
	const char          ATTRIBUTE_NAME_GROUP[]           = "group";        
	const char          ATTRIBUTE_NAME_INDEX[]           = "index"; 
	const char          ATTRIBUTE_NAME_DESC[]            = "desc";
	const char          ATTRIBUTE_NAME_SYNC[]            = "sync";
	const char          ATTRIBUTE_NAME_MODIFY[]          = "modify";
	const char          ATTRIBUTE_IM[]                   = "MID_IM";       

	const char          TAG_GROUP[]                      = "group";
	const char          TAG_USER[]                       = "user";

	const char          TAG_PRESENCE[]                   = "presence";     
	const char          TAG_TTYPE[]                      = "ttype";        
	const char          TAG_SHOW[]                       = "show";         
	const char          TAG_STATUS[]                     = "status";       

	const char          TAG_EXT[]                        = "ext";
	const char          TAG_SUBJECT[]                    = "subject";      
	const char          TAG_BODY[]                       = "body";         
	const char          TAG_ATTACHMENTS[]                = "attachments";  
	const char          TAG_ATTACHMENT[]                 = "attachment";   
	const char          TAG_NAME[]                       = "name";         
	const char          TAG_FORMAT[]                     = "format";       
	const char          TAG_ID[]                         = "id";           
	const char          TAG_SIZE[]                       = "size"; 
	const char          ATTRIBUTE_NAME_FONT[]            = "font";
	const char          ATTRIBUTE_NAME_TIME[]            = "time";
	const char          ATTRIBUTE_NAME_SOURCE[]          = "source";
	const char          ATTRIBUTE_NAME_AUTODISPLAY[]     = "auto-display"; 
	const char          ATTRIBUTE_NAME_AUTODOWNLOAD[]    = "auto-download";
	const char          ATTRIBUTE_NAME_DIR[]             = "dir";
	const char          ATTRIBUTE_NAME_URL[]             = "url";
	const char          ATTRIBUTE_NAME_AT[]              = "at";
	const char          ATTRIBUTE_NAME_AT_ID[]           = "atid";
	const char          ATTRIBUTE_NAME_CREATOR[]         = "creator";
	const char          ATTRIBUTE_NAME_ADDED[]           = "added";
	const char          ATTRIBUTE_NAME_ENCRYPT[]         = "encrypt";
	const char          ATTRIBUTE_NAME_PICWIDTH[]        = "picwidth";
	const char          ATTRIBUTE_NAME_PICHEIGHT[]       = "picheight";

	const char          ATTRIBUTE_NAME_IP[]              = "ip";
	const char          ATTRIBUTE_NAME_PORT[]            = "port";

	const char          TAG_TALKBEGIN[]                  = "talkbegin";    
	const char          ATTRIBUTE_MODULE_TALK[]          = "MID_TALK"; 
	const char          ATTRIBUTE_NAME_BEGINSEQ[]        = "beginseq";
	const char          TAG_AUDIODESC[]                  = "audiodesc";
	const char          ATTRIBUTE_NAME_BIT[]             = "bit";
	const char          ATTRIBUTE_NAME_CHAN[]            = "chan";
	const char          ATTRIBUTE_NAME_RATE[]            = "rate";
	const char          ATTRIBUTE_NAME_FRAME[]           = "frame";
	const char          ATTRIBUTE_NAME_BEGINTIME[]       = "begintime";
	const char          TAG_TOS[]                        = "tos";
	const char          TAG_TO[]                         = "to";
	const char          TAG_TALKEND[]                    = "talkend";      
	const char          ATTRIBUTE_NAME_ENDSEQ[]          = "endseq";
	const char          ATTRIBUTE_NAME_REASON[]          = "reason";
	const char          TAG_RECVEND[]                    = "recvend";      
	const char          TAG_RESEND[]                     = "resend";
	const char          ATTRIBUTE_NAME_AUDIOPACKET[]     = "audiopacket";

	const char          TAG_START[]                      = "start";
	const char          ATTRIBUTE_NAME_REQAUDIO[]        = "req_audio";
	const char          ATTRIBUTE_MODULE_VIDEO[]         = "MID_VIDEO";    
	const char          TAG_OK[]                         = "ok";
	const char          ATTRIBUTE_NAME_NEWID[]           = "newid";
	const char          ATTRIBUTE_NAME_VIDEOPARAM[]      = "videoparam";
	const char          ATTRIBUTE_NAME_WIDTH[]           = "width";
	const char          ATTRIBUTE_NAME_HEIGHT[]          = "height";
	const char          ATTRIBUTE_NAME_CODEC[]           = "codec";
	const char          ATTRIBUTE_NAME_CONFIG[]          = "config";
	const char          ATTRIBUTE_NAME_FPS[]             = "fps";
	const char          TAG_REJECT[]                     = "reject";       
	const char          TAG_REPORT[]                     = "report";   
	const char          TAG_STOP[]                       = "stop"; 
	const char          TAG_NOTIFY[]                     = "notify";

	const char          TAG_FIND[]                       = "find";
	const char          TAG_PHOTO[]                      = "photo";

	const char          TAG_MODIFY[]                     = "modify";

	const char          TAG_VIDEO[]                      = "video";
	const char          TAG_AUDIO[]                      = "audio";

	const char          ATTRIBUTE_MODULE_RTC[]           = "MID_RTC";
	const char          TAG_INVITE[]                     = "invite";
	const char          TAG_RINGING[]                    = "ringing";
	const char          TAG_BYE[]                        = "bye";
	const char          TAG_ACK[]                        = "ack";
	const char          TAG_REASON[]                     = "reason";
	const char          TAG_SDP[]                        = "sdp";
	const char          TAG_ICE_CANDIDATE[]              = "ice_candidate";

	const char          ATTRIBUTE_NAME_RECV[]            = "recv";
	const char          ATTRIBUTE_NAME_SEND[]            = "send";

	const char          ATTRIBUTE_NAME_ADDR[]            = "addr";

	const char          ATTRIBUTE_MSG[]                  = "MID_MSG";
	const char          TAG_MSG[]                        = "msg";
	const char          ATTRIBUTE_NAME_FTYPE[]           = "ftype";
	const char          ATTRIBUTE_NAME_COUNT[]           = "count";
	const char          ATTRIBUTE_NAME_TS[]              = "ts";
	const char          ATTRIBUTE_NAME_TS2[]             = "ts2";
	const char          ATTRIBUTE_NAME_TSNOW[]           = "tsnow";
	const char          ATTRIBUTE_NAME_SD[]              = "sd";
	const char          ATTRIBUTE_NAME_NUMBER[]          = "number";
	const char          ATTRIBUTE_NAME_TIMESTAMP[]       = "timestamp";


	const char          ATTRIBUTE_ORG[]                  = "MID_OS";
	const char          TAG_ORG[]                        = "os";
	const char          TAG_ORG_STRUCT[]                   = "s";


	// 
	const char          ATTRIBUTE_MODULE_DISCUSS[]       = "MID_DISCUSS";
	const char          TAG_DISCUSS[]                    = "discuss";

	const char          ATTRIBUTE_NAME_CREATE[]          = "create";
	const char          ATTRIBUTE_NAME_QUIT[]            = "quit";
	const char          ATTRIBUTE_NAME_ADD[]             = "add";
	const char          ATTRIBUTE_NAME_CHANGENAME[]      = "changename";
	const char          ATTRIBUTE_NAME_CHANGECARDNAME[]  = "changecardname";

	const char          TAG_LOGO_VERSION[]               = "logo_version";
	const char          TAG_LOGO[]                       = "logo";
	const char          TAG_ANNT[]                       = "annt";

	const char          ATTRIBUTE_MODULE_CONFIG[]        = "MID_CONF";
	const char          TAG_CONFIG[]                     = "config";
	const char          TAG_CONF1[]                      = "conf1";
	const char          TAG_CONF2[]                      = "conf2";
	const char          TAG_CONF3[]                      = "conf3"; 
	const char          TAG_CONF4[]                      = "conf4";
	const char          TAG_CONF5[]                      = "conf5";
	const char          TAG_CONF6[]                      = "conf6";
	const char          TAG_CONF7[]                      = "conf7";
	const char          TAG_CONF8[]                      = "conf8";
	const char          TAG_CONF9[]                      = "conf9";
	const char          TAG_GROUPS[]                     = "groups";
	const char          TAG_IDS[]                        = "ids";
	const char			TAG_SILENCE[]					 = "silence";	

	const char          TAG_NOTICE[]                     = "notice";
	const char          ATTRIBUTE_EVENT[]                = "event";
	const char		    EVENT_ID[]						 = "eid";

	const char          TAG_SEARCH[]                     = "search";
	const char          ATTRIBUTE_NAME_SEX[]             = "sex";
	const char          ATTRIBUTE_NAME_DEPART[]          = "depart"; 
	const char          ATTRIBUTE_NAME_PHONE[]           = "phone";
	const char          ATTRIBUTE_NAME_MORE[]            = "more";

	const char          TAG_TIP[]                        = "tip";
	const char          ATTRIBUTE_NAME_ACTION[]          = "action";

	const char          ATTRIBUTE_MODULE_INTERPHONE[]    = "MID_INTERPHONE";
	const char          TAG_INTERPHONES[]                = "interphones";
	const char          TAG_INTERPHONE[]                 = "interphone";
	const char          TAG_MEMBER[]                     = "member";
	const char          TAG_SPEAK[]                      = "speak";
	const char          ATTRIBUTE_NAME_SPEAKER[]         = "speaker";
	const char          ATTRIBUTE_NAME_ATTACHTYPE[]      = "attach_type";
	const char          ATTRIBUTE_NAME_ATTACHID[]        = "attach_id";
	const char          ATTRIBUTE_NAME_MEMBERCOUNT[]     = "member_count";

	const char          ATTRIBUTE_NAME_FULLTO[]          = "fullto";

	const char          ATTRIBUTE_ONLINE[]               = "MID_ONLINE";

	const char          TAG_WITHDRAW[]                   = "withdraw";
	const char          TAG_WITHDRAWS[]                  = "withdraws";
	const char          ATTRIBUTE_NAME_UID[]             = "uid";
	const char          ATTRIBUTE_NAME_WITHDRAWID[]      = "withdrawId";

	const char          ATTRIBUTE_NAME_CARDNAME[]        = "cardname";

	const char		    ATTRIBUTE_MODLE_IOS_PUSH[]		 = "MID_IOSPUSH";
	const char		    ATTRIBUTE_PUSH_NOTIFICATION[]	 = "push_notification";
	const char		    ATTRIBUTE_SILENCE[]				 = "silence";
}