#ifndef _DETAIL_ITEM_H_
#define _DETAIL_ITEM_H_

#include <QString>
#include <QImage>
#include <QVariantMap>

namespace bean
{
	enum DetailDataRole
	{
		DETAIL_UID = 1,
		DETAIL_NAME,
		DETAIL_MESSAGE,
		DETAIL_SEX,
		DETAIL_BIRTHDAY,
		DETAIL_PHONE1,
		DETAIL_PHONE2,
		DETAIL_PHONE3,
		DETAIL_EMAIL,
		
		DETAIL_DEPART,
		DETAIL_ORGANIZATION,
		DETAIL_DUTY,
		DETAIL_AREA,
		DETAIL_JOBDESC,

		DETAIL_VERSION,
		DETAIL_UPDATETIME,

		DETAIL_PHOTO,

		DETAIL_DISABLED
	};

    class DetailItem
    {
	public:
		static const int DetailItem::max_photo_size  = 110;
		static const int DetailItem::invalid_version = -1;

    public:
		QString uid() const;
		void setUid(const QString &uid);

		QString name() const;
		void setName(const QString &name);

		QString message() const;
		void setMessage(const QString &message);

		int sex() const;
		void setSex(int sex);

		QString birthday() const;
		void setBirthday(const QString &birthday);

		QString phone1() const;
		void setPhone1(const QString &phone1);

		QString phone2() const;
		void setPhone2(const QString &phone2);

		QString phone3() const;
		void setPhone3(const QString &phone3);

		QString email() const;
		void setEmail(const QString &email);

		QString depart() const;
		void setDepart(const QString &depart);

		QString organization() const;
		void setOrganization(const QString &organization);

		QString duty() const;
		void setDuty(const QString &duty);

		QString area() const;
		void setArea(const QString &area);

		QString jobDesc() const;
		void setJobDesc(const QString &jobDesc);

		bool isDisabled() const;
		void setDisabled(bool disabled);

		int version() const;
		void setVersion(int ver);

		QString updateTime() const;
		void setUpdateTime(const QString &updateTime);

		QString data(DetailDataRole role) const;
		void setData(DetailDataRole role, QVariant data);

		static const char* detailDataRole2String(DetailDataRole role);

		DetailItem *clone() const;

	public:
		void fromDBMap(const QVariantMap &dbMap);
		QVariantMap toDBMap() const;

	private:
		DetailItem();
        
	private:
		QString m_uid;
		QString m_name;    // 姓名

		QString m_message; // 个性签名
		int     m_sex;     // 性别   0:女,  1:男,  9:未知
		QString m_birthday;// 生日
		QString m_phone1;  // 手机
		QString m_phone2;  // 其它手机
		QString m_phone3;  // 电话
		QString m_email;   // 电子邮件
		
		QString m_depart;  // 部门
		QString m_organization; // 单位
		QString m_duty;    // 职位
		QString m_area;    // 所在区域
		QString m_jobDesc; // 工作内容

		int     m_disabled; // 是否注销， 0：没注销 1：已注销

		int     m_version;
		QString m_updateTime;

		friend class DetailItemFactory;
    };

	class DetailItemFactory
	{
	public:
		static DetailItem* createItem()
		{
			return new DetailItem();
		}
	};
}


#endif // _DETAIL_ITEM_H_
