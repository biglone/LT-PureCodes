#include "DetailItem.h"
#include "Account.h"

static const char *field_uid             = "uid";
static const char *field_name            = "name";

static const char *field_message         = "message";
static const char *field_sex             = "sex";
static const char *field_birthday        = "birthday";
static const char *field_phone1          = "phone1";
static const char *field_phone2          = "phone2";
static const char *field_phone3          = "phone3";
static const char *field_email           = "email";

static const char *field_depart          = "depart";
static const char *field_organization    = "organization";
static const char *field_duty            = "duty";
static const char *field_area            = "area";
static const char *field_jobdesc         = "jobdesc";

static const char *field_disabled        = "disabled";

static const char *field_version         = "version";
static const char *field_updatetime      = "updatetime";

static void registerMetatype()
{
	static bool isInit = false;
	if (!isInit)
	{
		qRegisterMetaType<bean::DetailItem*>("bean::DetailItem*");
		isInit = true;
	}
}

namespace bean
{
	DetailItem::DetailItem() : m_version(invalid_version), m_sex(9), m_disabled(0)
	{
		registerMetatype();
	}

	QString DetailItem::uid() const
	{
		return m_uid;
	}

	void DetailItem::setUid(const QString &uid)
	{
		m_uid = uid;
	}

	QString DetailItem::name() const
	{
		return m_name;
	}

	void DetailItem::setName(const QString &name)
	{
		m_name = name;
	}

	QString DetailItem::message() const
	{
		return m_message;
	}

	void DetailItem::setMessage(const QString &message)
	{
		m_message = message;
	}

	int DetailItem::sex() const
	{
		return m_sex;
	}

	void DetailItem::setSex(int sex)
	{
		m_sex = sex;
	}

	QString DetailItem::birthday() const
	{
		return m_birthday;
	}

	void DetailItem::setBirthday(const QString &birthday)
	{
		m_birthday = birthday;
	}

	QString DetailItem::phone1() const
	{
		return m_phone1;
	}

	void DetailItem::setPhone1(const QString &phone1)
	{
		m_phone1 = phone1;
	}

	QString DetailItem::phone2() const
	{
		return m_phone2;
	}

	void DetailItem::setPhone2(const QString &phone2)
	{
		m_phone2 = phone2;
	}

	QString DetailItem::phone3() const
	{
		return m_phone3;
	}

	void DetailItem::setPhone3(const QString &phone3)
	{
		m_phone3 = phone3;
	}

	QString DetailItem::email() const
	{
		return m_email;
	}

	void DetailItem::setEmail(const QString &email)
	{
		m_email = email;
	}

	QString DetailItem::depart() const
	{
		return m_depart;
	}

	void DetailItem::setDepart(const QString &depart)
	{
		m_depart = depart;
	}

	QString DetailItem::organization() const
	{
		return m_organization;
	}

	void DetailItem::setOrganization(const QString &organization)
	{
		m_organization = organization;
	}

	QString DetailItem::duty() const
	{
		return m_duty;
	}

	void DetailItem::setDuty(const QString &duty)
	{
		m_duty = duty;
	}

	QString DetailItem::area() const
	{
		return m_area;
	}

	void DetailItem::setArea(const QString &area)
	{
		m_area = area;
	}

	QString DetailItem::jobDesc() const
	{
		return m_jobDesc;
	}

	void DetailItem::setJobDesc(const QString &jobDesc)
	{
		m_jobDesc = jobDesc;
	}

	bool DetailItem::isDisabled() const
	{
		return (m_disabled == 1) ? true : false;
	}

	void DetailItem::setDisabled(bool disabled)
	{
		m_disabled = (disabled ? 1 : 0);
	}

	int DetailItem::version() const
	{
		return m_version;
	}

	void DetailItem::setVersion(int ver)
	{
		m_version = ver;
	}

	QString DetailItem::updateTime() const
	{
		return m_updateTime;
	}

	void DetailItem::setUpdateTime(const QString &updateTime)
	{
		m_updateTime = updateTime;
	}

	QString DetailItem::data(DetailDataRole role) const
	{
		switch (role)
		{
		case DETAIL_UID:
			return m_uid;
		case DETAIL_NAME:
			return m_name;

		case DETAIL_MESSAGE:
			return m_message;
		case DETAIL_SEX:
			return QString::number(m_sex);
		case DETAIL_BIRTHDAY:
			return m_birthday;
		case DETAIL_PHONE1:
			return m_phone1;
		case DETAIL_PHONE2:
			return m_phone2;
		case DETAIL_PHONE3:
			return m_phone3;
		case DETAIL_EMAIL:
			return m_email;
		
		case DETAIL_DEPART:
			return m_depart;
		case DETAIL_DUTY:
			return m_duty;
		case DETAIL_AREA:
			return m_area;
		case DETAIL_JOBDESC:
			return m_jobDesc;

		case DETAIL_DISABLED:
			return QString::number(m_disabled);

		case DETAIL_VERSION:
			return QString::number(m_version);
		case DETAIL_UPDATETIME:
			return m_updateTime;

		default:
			return QString();
		}
	}

	void DetailItem::setData(DetailDataRole role, QVariant data)
	{
		switch (role)
		{
		case DETAIL_UID:
			setUid(data.toString());
			break;
		case DETAIL_NAME:
			setName(data.toString());
			break;

		case DETAIL_MESSAGE:
			setMessage(data.toString());
			break;
		case DETAIL_SEX:
			setSex(data.toString().toInt());
			break;
		case DETAIL_BIRTHDAY:
			setBirthday(data.toString());
			break;
		case DETAIL_PHONE1:
			setPhone1(data.toString());
			break;
		case DETAIL_PHONE2:
			setPhone2(data.toString());
			break;
		case DETAIL_PHONE3:
			setPhone3(data.toString());
			break;
		case DETAIL_EMAIL:
			setEmail(data.toString());
			break;
		
		case DETAIL_DEPART:
			setDepart(data.toString());
			break;
		case DETAIL_DUTY:
			setDuty(data.toString());
			break;
		case DETAIL_AREA:
			setArea(data.toString());
			break;
		case DETAIL_JOBDESC:
			setJobDesc(data.toString());
			break;

		case DETAIL_DISABLED:
			setDisabled(data.toString().toInt() == 1);
			break;

		case DETAIL_VERSION:
			setVersion(data.toString().toInt());
			break;
		case DETAIL_UPDATETIME:
			setUpdateTime(data.toString());
			break;
		
		default:
			break;
		}
	}

	const char* DetailItem::detailDataRole2String(DetailDataRole role)
	{		
		switch (role)
		{
		case DETAIL_UID:
			return "id";
		case DETAIL_NAME:        // 姓名
			return "name";       
		
		case DETAIL_MESSAGE:     // 个性签名
			return "message";
		case DETAIL_SEX:         // 性别   0:女,  1:男,  9:未知
			return "sex";
		case DETAIL_BIRTHDAY:    // 生日
			return "birthday";
		case DETAIL_PHONE1:      // 电话1
			return "phone1";
		case DETAIL_PHONE2:      // 电话2
			return "phone2";
		case DETAIL_PHONE3:      // 电话3
			return "phone3";
		case DETAIL_EMAIL:       // 电子邮件
			return "email";
		
		case DETAIL_DEPART:      // 部门
			return "depart";
		case DETAIL_DUTY:        // 职位，职务
			return "duty";
		case DETAIL_AREA:        // 所在区域
			return "area";
		case DETAIL_JOBDESC:     // 工作内容
			return "jobdesc";

		case DETAIL_VERSION:
			return "version";        // 版本号
		case DETAIL_UPDATETIME:
			return "updatetime";     // 更新时间
		
		case DETAIL_PHOTO:
			return "photo";          // 头像

		case DETAIL_DISABLED:
			return "disabled";       // 是否注销
		
		}
		return "";
	}

	DetailItem *DetailItem::clone() const
	{
		DetailItem *newDetailItem = DetailItemFactory::createItem();
		
		newDetailItem->setUid(m_uid);
		newDetailItem->setName(m_name);
		
		newDetailItem->setMessage(m_message);
		newDetailItem->setSex(m_sex);
		newDetailItem->setBirthday(m_birthday);
		newDetailItem->setPhone1(m_phone1);
		newDetailItem->setPhone2(m_phone2);
		newDetailItem->setPhone3(m_phone3);
		newDetailItem->setEmail(m_email);
		
		newDetailItem->setDepart(m_depart);
		newDetailItem->setOrganization(m_organization);
		newDetailItem->setDuty(m_duty);
		newDetailItem->setArea(m_area);
		newDetailItem->setJobDesc(m_jobDesc);
		newDetailItem->setDisabled(this->isDisabled());
	
		newDetailItem->setVersion(m_version);
		newDetailItem->setUpdateTime(m_updateTime);

		return newDetailItem;
	}

	void DetailItem::fromDBMap(const QVariantMap &dbMap)
	{
		m_uid = dbMap.value(field_uid, "").toString();
		m_name = dbMap.value(field_name, "").toString();

		m_message = dbMap.value(field_message, "").toString();
		QString strSex = dbMap.value(field_sex, "9").toString();
		if (strSex.isEmpty())
		{
			m_sex = 9;
		}
		else
		{
			bool ok = false;
			m_sex = strSex.toInt(&ok);
			if (!ok)
			{
				m_sex = 9;
			}
		}
		m_birthday = dbMap.value(field_birthday, "").toString();
		m_phone1 = dbMap.value(field_phone1, "").toString();
		m_phone2 = dbMap.value(field_phone2, "").toString();
		m_phone3 = dbMap.value(field_phone3, "").toString();
		m_email = dbMap.value(field_email, "").toString();
		
		m_depart = dbMap.value(field_depart, "").toString();
		m_organization = dbMap.value(field_organization, "").toString();
		m_duty = dbMap.value(field_duty, "").toString();
		m_area = dbMap.value(field_area, "").toString();
		m_jobDesc = dbMap.value(field_jobdesc, "").toString();

		m_disabled = dbMap.value(field_disabled, 0).toInt();
		
		m_version = dbMap.value(field_version, -1).toInt();
		m_updateTime = dbMap.value(field_updatetime, "").toString();
	}

	QVariantMap DetailItem::toDBMap() const
	{
		QVariantMap dbMap;
		dbMap[field_uid] = m_uid;
		dbMap[field_name] = m_name;

		dbMap[field_message] = m_message;
		dbMap[field_sex] = m_sex;
		dbMap[field_birthday] = m_birthday;
		dbMap[field_phone1] = m_phone1;
		dbMap[field_phone2] = m_phone2;
		dbMap[field_phone3] = m_phone3;
		dbMap[field_email] = m_email;
		
		dbMap[field_depart] = m_depart;
		dbMap[field_organization] = m_organization;
		dbMap[field_duty] = m_duty;
		dbMap[field_area] = m_area;
		dbMap[field_jobdesc] = m_jobDesc;

		dbMap[field_disabled] = m_disabled;

		dbMap[field_version] = m_version;
		dbMap[field_updatetime] = m_updateTime;
		
		return dbMap;
	}

}
