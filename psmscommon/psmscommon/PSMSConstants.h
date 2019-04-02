#ifndef _PSMSCOMMON_CONSTANTS_H_INCLUDED
#define _PSMSCOMMON_CONSTANTS_H_INCLUDED

namespace psmscommon
{

static const int BUFFER_LEN                     = 1024*2;       /// 正常缓冲长度
static const int BUFFER_LEN_SHORT               = 1024;         /// 小缓冲长度
static const int BUFFER_LEN_SEQ                 = 18;           /// 序列号长度

static const int SP_RELOGIN_INTERVAL            = 10;           /// SP重新登录间隔时间(s)

static const char* SERVICE_LOG                  = "log@";

static const char* MODULE_BASE                  = "base";
static const char* PSG_NAME                     = "psg";

static const char* TAG_MESSAGE                  = "message";
static const char* TAG_HEARTBEAT                = "heartbeat";
static const char* TAG_LOGIN                    = "login";
static const char* TAG_EXIST                    = "exist";
static const char* TAG_LOG                      = "log";

static const char* ATTRIBUTE_TYPE               = "type";
static const char* ATTRIBUTE_SEQ                = "seq";
static const char* ATTRIBUTE_TO                 = "to";
static const char* ATTRIBUTE_FROM               = "from";
static const char* ATTRIBUTE_MODULE             = "module";
static const char* ATTRIBUTE_DOMAIN             = "domain";
static const char* ATTRIBUTE_ERRCODE            = "errcode";
static const char* ATTRIBUTE_ERRMSG             = "errmsg";
static const char* ATTRIBUTE_NAME               = "name";
static const char* ATTRIBUTE_ID                 = "id";

static const char* VALUE_REQUEST                = "request";
static const char* VALUE_RESPONSE               = "response";
static const char* VALUE_NOTIFICATION           = "notification";

} // namespace psmscommon

#endif //_PSMSCOMMON_CONSTANTS_H_INCLUDED
