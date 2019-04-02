#ifndef _SETTING_CONSTANTS_H_
#define _SETTING_CONSTANTS_H_

/// 公网环境
extern const char OFFICIAL_CONFIG_MANAGER_URL[];

/// 测试环境
extern const char TEST_CONFIG_MANAGER_URL[];

/// Ptt音频的配置
extern const char CONFIG_AUDIODESC_TYPE[];
extern const int  CONFIG_AUDIODESC_CHAN;
extern const int  CONFIG_AUDIODESC_BIT;
extern const int  CONFIG_AUDIODESC_RATE;
extern const int  CONFIG_AUDIODESC_FRAME;

/// 视频采集配置
extern const int  CONFIG_VIDEO_DEVICEID;
extern const int  CONFIG_VIDEO_WIDTH;
extern const int  CONFIG_VIDEO_HEIGHT;
extern const int  CONFIG_VIDEO_FPS;
extern const char CONFIG_VIDEO_CODEC[];

/// 提示音路径
extern const char CONFIG_SEND_BEEP_PATH[];
extern const char CONFIG_RECV_BEEP_PATH[];
extern const char CONFIG_AUDIO_BEEP_PATH[];

// 全局设置
extern const char COMMONSET_TITLE[];
extern const char COMMONSET_SSL[];
extern const char COMMONSET_LOGIN_ADDRESS[];
extern const char COMMONSET_TRANSFER_ADDRESS[];
extern const char COMMONSET_UPDATE_URL[];
extern const char COMMONSET_UPDATE_DESC[];
extern const char COMMONSET_AUDIO_DISABLED[];
extern const char COMMONSET_VIDEO_DISABLED[];
extern const char COMMONSET_ROAMING_MSG_DISABLED[];
extern const char COMMONSET_LOGIN_LOAD_BALANCE[];
extern const char COMMONSET_INTRODUCTION_VIEW_TYPE[];
extern const char COMMONSET_SUBSCRIPTION_DISABLED[];
extern const char COMMONSET_LINK_ITEMS[];
extern const char COMMONSET_MAX_DISCUSS_MEMBER_COUNT[];
extern const char COMMONSET_TRACKER_SERVER[];
extern const char COMMONSET_FASTDFS_ENABLED[];
extern const char COMMONSET_ROSTER_SMALL_AVATAR[];
extern const char COMMONSET_OS_LOAD_ALL[];
extern const char COMMONSET_OFFLINE_SYNC_MSG_ENABLED[];
extern const char COMMONSET_MSG_ENCRYPT[];
extern const char COMMONSET_MSG_ENCRYPT_SEED[];
extern const char COMMONSET_INTER_PHONE_DISABLED[];

#endif // _SETTING_CONSTANTS_H_
