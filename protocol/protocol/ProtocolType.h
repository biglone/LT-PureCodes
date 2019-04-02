#ifndef _PROTOCOLTYPE_H_
#define _PROTOCOLTYPE_H_
#include <string>

namespace protocol
{
enum RequestType 
{
	// base
    Request_Base_Login           = 0x00000000,
	Request_Base_Logout          = 0x00000001,
    Request_Base_Update          = 0x00000002,
    Request_Base_Update_Plugins  = 0x00000003,
    Request_Base_Download        = 0x00000004,
	Request_Base_Timesync        = 0x00000005,
	Request_Base_Password        = 0x00000006,
	Request_Base_Feedback        = 0x00000007,

    // SP-IM
    // 名册管理
	Request_IM_Roster          = 0x00010000,
	
    // 群组管理
    Request_IM_Group           = 0x00010001,
    
	// 查询
	Request_IM_Find            = 0x00010002,

	// 修改个人信息
	Request_IM_Modify          = 0x00010003,

	// 位置信息提交
	Request_Location_Submit    = 0x00020000,

	// 离线消息
	Request_Msg_Offline        = 0x00030000,

	// 历史消息
	Request_Msg_History        = 0x00030001,

	// 发送消息
	Request_Msg_Send           = 0x00030002,

	// 组织结构
	Request_OS_Org             = 0x00040000,

	// 讨论组
	Request_Discuss_Discuss    = 0x00050000,

	// 个人设置
	Request_Config_Config      = 0x00060000,

	// 上报TS
	Request_Report_Ts          = 0x00070000,

	// 搜索
	Request_IM_Search          = 0x00080000,

	// 实时对讲
	Request_Interphone_Interphone = 0x00090000,

	// 上报在线
	Request_Report_Online      = 0x000A0000,

	// 消息撤回
	Request_Message_Withdraw   = 0x000B0000,
	Request_Withdraw_Sync      = 0x000B0001,

	// 群组修改备注
	Request_Group_CardName     = 0x000C0000
};

enum NotificationType
{
	// PSG
	KICK                = 0x00000000,
	RELOGIN             = 0x00000001,

	// IM
	PRESENCE            = 0x00000010,
	MESSAGE             = 0x00000011,
	HEARTBEAT           = 0x00000012,
	TIP                 = 0x00000013,

	// TALK
	TALK_TALKBEGIN      = 0x00000020,
	TALK_TALKEND        = 0x00000021,
	TALK_RECVEND        = 0x00000022,
	TALK_RESEND         = 0x00000023,

	// VIDEO
	VIDEO_START         = 0x00000030,
	VIDEO_OK            = 0x00000031,
	VIDEO_REJECT        = 0x00000032,
	VIDEO_REPORT        = 0x00000033,
	VIDEO_STOP          = 0x00000034,

	// session
	SESSION_INVITE      = 0x00000040,
	SESSION_RINGING     = 0x00000041,
	SESSION_OK          = 0x00000042,
	SESSION_REJECT      = 0x00000043,
	SESSION_MODIFY      = 0x00000044,
	SESSION_BYE         = 0x00000045,
	SESSION_REPORT      = 0x00000046,
	SESSION_ACK         = 0x00000047,
	SESSION_NOTIFY      = 0x00000048,
	SESSION_SDP         = 0x00000049,
	SESSION_ICE         = 0X0000004a,

	// discuss
	DISCUSS             = 0x00000050,

	// change notice
	CHANGENOTICE        = 0x00000060, 

	// interphone
	INTERPHONE          = 0x00000070,

	// roster
	ROSTER              = 0x00000080,

	// message withdraw
	MESSAGE_WITHDRAW    = 0x00000090,

	// group notification
	GROUP               = 0x000000a0
};
}

#endif //_PROTOCOLTYPE_H_
