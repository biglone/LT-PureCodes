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
		QString m_name;    // ����

		QString m_message; // ����ǩ��
		int     m_sex;     // �Ա�   0:Ů,  1:��,  9:δ֪
		QString m_birthday;// ����
		QString m_phone1;  // �ֻ�
		QString m_phone2;  // �����ֻ�
		QString m_phone3;  // �绰
		QString m_email;   // �����ʼ�
		
		QString m_depart;  // ����
		QString m_organization; // ��λ
		QString m_duty;    // ְλ
		QString m_area;    // ��������
		QString m_jobDesc; // ��������

		int     m_disabled; // �Ƿ�ע���� 0��ûע�� 1����ע��

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
