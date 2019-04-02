#ifndef GLOBALNOTIFICATIONDETAIL_H
#define GLOBALNOTIFICATIONDETAIL_H

#include <QVariantMap>

class GlobalNotificationDetail
{
public:
	GlobalNotificationDetail();
	~GlobalNotificationDetail();

	QString id() const { return m_id; }
	void setId(const QString id) { m_id = id; }

	QString name() const { return m_name; }
	void setName(const QString &name) { m_name = name; }

	int type() const { return m_type; }
	void setType(int type) { m_type = type; }

	QString logo() const { return m_logo; }
	void setLogo(const QString &logo) { m_logo = logo; }

	QString num() const { return m_num; }
	void setNum(const QString &num) { m_num = num; }

	QString introduction() const { return m_introduction; }
	void setIntroduction(const QString &introduction) { m_introduction = introduction; }

	int special() const { return m_special; }
	void setSpecial(int special) { m_special = special; }

	QVariantMap toDBMap() const;
	void fromDBMap(const QVariantMap &vm);

	bool isValid() const;

private:
	QString m_id;
	QString m_name;
	int     m_type;
	QString m_logo;
	QString m_num;
	QString m_introduction;
	int     m_special;
};

#endif // GLOBALNOTIFICATIONDETAIL_H
