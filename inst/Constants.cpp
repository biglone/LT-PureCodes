#include "..\version.h"
#include "Constants.h"

const char LOCAL_COMM_SERVER_NAME[]        = "lingtalk_server";

const char APP_ID[]                        = "{D23DD6E0-D830-4260-A24C-3308B2DA2E52}";
const char ORG_DOMAIN[]                    = "www.lingtalk.cn";
const char ORG_NAME[]                      = "LingTalk";

const char APP_VERSION[]                   = VER_TOSTRING(VER_MAJOR, VER_MINOR, VER_REVISION, VER_BUILD);

const char APP_NAME[]                      = "LingTalk";
const char APP_DEAMON_NAME[]               = "LingTalkDeamon";
const char APP_EXE_NAME[]                  = "LingTalkInst.exe";
const char APP_DEAMON_EXE_NAME[]           = "LingTalk.exe";

// online 
const char STATUS_ONLINE_KEY[]             = "online";   // 有空
const char STATUS_OFFLINE_KEY[]            = "offline";  // 离线
const char STATUS_DND_KEY[]                = "dnd";      // 忙碌
const char STATUS_XA_KEY[]                 = "xa";       // 离开

// message related
const char MAP_DIALOG_KEY_FORMAT[]         = "%1%2%3"; // message.type - id
const char MAP_KEY_SEPARATION              = 0x1B;

// encrypt type string
const char CONFIG_ENCRYPT_NONE[]           = "none";
// const char CONFIG_ENCRYPT_RC4[]            = "rc4";
const char CONFIG_ENCRYPT_TLS12[]          = "TLSv1.2";
const char CONFIG_ENCRYPT_TLS11[]          = "TLSv1.1";
const char CONFIG_ENCRYPT_TLS1[]           = "TLSv1";
const char CONFIG_ENCRYPT_SSL23[]          = "SSLv23";
const char CONFIG_ENCRYPT_SSL3[]           = "SSLv3";
const char CONFIG_ENCRYPT_SSL2[]           = "SSLv2";

// roster add robot
const char ROSTER_ADD_MESSAGE_ID[]         = "ROSTERADD_437D1C5D7B584A129849B72F304775AD";

// subscription message
const char SUBSCRIPTION_ROSTER_ID[]        = "SUBSCRIPTION_7DD4B0853A0F4644A953EAC9AC16DFC1";

// globalNotification message
const char GLOBALNOTIFICATION_ROSTER_ID[]  = "GLOBALNOTIFICATION_73046B4316F340779D2B07A8B86248CC";


