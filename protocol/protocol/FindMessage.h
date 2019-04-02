#ifndef _FINDMESSAGE_H_
#define _FINDMESSAGE_H_

#include <string>
#include <map>
#include <list>

#include "net/Request.h"
#include "protocol/Response.h"

#include "protocol_global.h"

namespace protocol
{
	class PROTOCOL_EXPORT FindMessage
	{
	public:
		enum FindType
		{
			// user
			Find_User_Info           = 0x00000000,

			Find_User_Detail         = 0x00000001,
			Find_User_Photo          = 0x00000002,
			Find_User_DetailPhoto    = Find_User_Detail|Find_User_Photo,

			Find_User_Version        = 0x00000004,

			// group
			Find_Group_Presence      = 0x00000010,
		};


		static std::string FindTypeToString(FindType type);
		static FindType    FindTypeFromString(const std::string& type);

		typedef std::map<std::string, std::string> FindResult;
		typedef std::map<std::string, FindResult>  MapFindResult;


		//struct FindResult
		//{
		//	std::string           id;          // id号
		//	std::string           duty;        // 职位，职务
		//	std::string           phone1;      // 电话1
		//	std::string           phone2;      // 电话2
		//	std::string           phone3;      // 电话3
		//	std::string           email;       // 电子邮件
		//	std::string           photo;       // 头像
		//	std::string           sex;         // 性别   0:女,  1:男,  9:未知
		//	std::string           nickName;    // 昵称
		//	std::string           homePage;    // 个人主页
		//	std::string           area;        // 所在区域
		//	std::string           birthday;    // 生日
		//	std::string           remark;      // 备注
		//	std::string           message;     // 个性签名
		//	std::string           interest;    // 兴趣爱好
		//	std::string           jobDesc;     // 工作内容
		//	std::string           agname;      // 绰号
		//	std::string           stature;     // 身高
		//	std::string           weight;      // 体重
		//};



		class PROTOCOL_EXPORT FindRequest : public net::Request
		{
		public:
			FindRequest(FindType type = Find_User_Info);

			void addId(const std::string& id);

			virtual int getType();

			int getFindType() const;

			std::string getBuffer();

		protected:
			// 有结果
			virtual void onResponse(net::RemoteResponse* response);
			
		private:
			FindMessage::FindType    m_eType;

			std::list<std::string>   m_listIds;
		};

		class PROTOCOL_EXPORT FindRespone : public Response
		{
		public:
			FindRespone(net::RemoteResponse* pRr);
			bool Parse();

			int getFindType() const;
			MapFindResult getFindUserResult() const;
			std::map<std::string, MapFindResult> getFindGroupResult() const;

		private:
			MapFindResult parseItem(void* pIks);

		private:
			FindMessage::FindType                  m_eType;      // 查询类型
			MapFindResult                          m_mapUser;    // user 查询结果
			std::map<std::string, MapFindResult>   m_mapGroup;   // group 查询结果
		};

	private:
		FindMessage() {}

	};
}

#endif