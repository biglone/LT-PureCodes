#ifndef _CONSTANTS_INCLUDED
#define _CONSTANTS_INCLUDED

extern const char LOCAL_COMM_SERVER_NAME[];

extern const char APP_ID[];
extern const char ORG_DOMAIN[];
extern const char ORG_NAME[];

extern const char APP_VERSION[];

extern const char APP_NAME[];
extern const char APP_DEAMON_NAME[];
extern const char APP_EXE_NAME[];
extern const char APP_DEAMON_EXE_NAME[];

// online 
extern const char STATUS_ONLINE_KEY[];   // 有空
extern const char STATUS_OFFLINE_KEY[];  // 离线
extern const char STATUS_DND_KEY[];      // 忙碌
extern const char STATUS_XA_KEY[];       // 离开

// message related
extern const char MAP_DIALOG_KEY_FORMAT[]; // message.type - id
extern const char MAP_KEY_SEPARATION;

// encrypt type string
extern const char CONFIG_ENCRYPT_NONE[];
// extern const char CONFIG_ENCRYPT_RC4[];
extern const char CONFIG_ENCRYPT_TLS12[];
extern const char CONFIG_ENCRYPT_TLS11[];
extern const char CONFIG_ENCRYPT_TLS1[];
extern const char CONFIG_ENCRYPT_SSL23[];
extern const char CONFIG_ENCRYPT_SSL3[];
extern const char CONFIG_ENCRYPT_SSL2[];

enum EncryptType
{
	Encrypt_None = 0,
	// Encrypt_RC4, -- omit
	Encrypt_TLSV12 = 2,
	Encrypt_TLSV11,
	Encrypt_TLSV1,
	Encrypt_SSLV23,
	Encrypt_SSLV3,
	Encrypt_SSLV2
};

// roster add robot
extern const char ROSTER_ADD_MESSAGE_ID[];

// subscription message
extern const char SUBSCRIPTION_ROSTER_ID[];

// globalNotification message
extern const char GLOBALNOTIFICATION_ROSTER_ID[];

// local communication codes
namespace ns_local_comm 
{
	enum MessageType 
	{
		MsgTypeNone = 0,
		MsgShortcut,
		MsgUpdateCheck,
		MsgApplication
	};

	enum ShortcutMsgCode
	{
		SetShortcut = 0x01,
		GetMsg = 0x02,
		ShowHide = 0x04,
		SnapShot = 0x08
	};

	enum UpdateCheckMsgCode
	{
		ManualUpdateCheck = 0x01,
		AutoUpdateCheck
	};

	enum ApplicationMsgCode
	{
		AppClose = 0x01,
		AppVersion,
		AppLoadGlobalSetting,
		AppSaveGlobalSetting,
		AppLoadAccounts,
		AppSaveAccounts,
		AppSetLoginAccount,
		AppQueryAccountLogined,
		AppSetLogoutAccount,
		AppSetUpdate,
		AppQueryUpdate
	};

	const unsigned int MSG_ERROR_CODE = 0;
};

#endif //_CONSTANTS_INCLUDED
