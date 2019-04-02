#include "SettingConstants.h"

/// 公网环境
/// 后台的IP, Port
const char OFFICIAL_CONFIG_MANAGER_URL[]   = "https://api.lingtalk.cn:8443/pmm";

/// 测试环境
const char TEST_CONFIG_MANAGER_URL[]       = "http://58.211.187.150:28083/pmm";

/// 音频的配置
const char  CONFIG_AUDIODESC_TYPE[]        = "opus";
const int   CONFIG_AUDIODESC_CHAN          = 1;
const int   CONFIG_AUDIODESC_BIT           = 16;
const int   CONFIG_AUDIODESC_RATE          = 8000;
const int   CONFIG_AUDIODESC_FRAME         = 40;

/// 视频的配置
const int   CONFIG_VIDEO_DEVICEID          = 0;
const int   CONFIG_VIDEO_WIDTH             = 352;
const int   CONFIG_VIDEO_HEIGHT            = 288;
const int   CONFIG_VIDEO_FPS               = 10;
const char  CONFIG_VIDEO_CODEC[]           = "h264";

/// 提示音路径
const char CONFIG_SEND_BEEP_PATH[]         = "./Misc/sounds/sound_1.wav";
const char CONFIG_RECV_BEEP_PATH[]         = "./Misc/sounds/sound_2.wav";
const char CONFIG_AUDIO_BEEP_PATH[]        = "./Misc/sounds/sound_3.wav";

// 全局配置
const char COMMONSET_TITLE[]                        = "title";
const char COMMONSET_SSL[]                          = "ssl";
const char COMMONSET_LOGIN_ADDRESS[]                = "login_address";
const char COMMONSET_TRANSFER_ADDRESS[]             = "transfer_address";
const char COMMONSET_UPDATE_URL[]                   = "update_url";
const char COMMONSET_UPDATE_DESC[]                  = "update_desc";
const char COMMONSET_AUDIO_DISABLED[]               = "audio_disabled";
const char COMMONSET_VIDEO_DISABLED[]               = "video_disabled";
const char COMMONSET_ROAMING_MSG_DISABLED[]         = "roaming_msg_disabled";
const char COMMONSET_LOGIN_LOAD_BALANCE[]           = "login_load_balance";
const char COMMONSET_INTRODUCTION_VIEW_TYPE[]       = "introduction_view_type";
const char COMMONSET_SUBSCRIPTION_DISABLED[]        = "subscription_disabled";
const char COMMONSET_LINK_ITEMS[]                   = "link_items";
const char COMMONSET_MAX_DISCUSS_MEMBER_COUNT[]     = "max_discuss_member_count";
const char COMMONSET_TRACKER_SERVER[]               = "tracker_server";
const char COMMONSET_FASTDFS_ENABLED[]              = "fastdfs_enabled"; 
const char COMMONSET_ROSTER_SMALL_AVATAR[]          = "roster_small_avatar";
const char COMMONSET_OS_LOAD_ALL[]                  = "os_load_all";
const char COMMONSET_OFFLINE_SYNC_MSG_ENABLED[]     = "offline_sync_msg_enabled";
const char COMMONSET_MSG_ENCRYPT[]                  = "msg_encrypt";
const char COMMONSET_MSG_ENCRYPT_SEED[]             = "msg_encrypt_seed";
const char COMMONSET_INTER_PHONE_DISABLED[]         = "inter_phone_disabled";
