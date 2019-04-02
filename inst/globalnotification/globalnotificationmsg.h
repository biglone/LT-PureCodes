#ifndef GLOBALNOTIFICATIONMSG_H
#define GLOBALNOTIFICATIONMSG_H

#include <QString>
#include <QVariantMap>

class GlobalNotificationMsg
{
public:
	static const int kTypeText      = 1;
	static const int kTypeImageText = 2;
	static const int kTypeImage     = 3;
	static const int kTypeAttach    = 4;

public:
	GlobalNotificationMsg();
	~GlobalNotificationMsg();

	quint64 innerId() const { return m_innerId; }
	void setInnerId(quint64 innerId) {m_innerId = innerId;}

	QString id() const { return m_id; }
	void setId(const QString id) { m_id = id; }

	int type() const { return m_type; }
	void setType(int type) { m_type = type; }

	QString content() const { return m_content; }
	void setContent(const QString &content) { m_content = content; }

	QString userId() const { return m_userId; }
	void setUserId(const QString &userId) { m_userId = userId; }

	QString globalNotificationId() const { return m_globalNotificationId; }
	void setGlobalNotificationId(const QString &globalNotificationId) { m_globalNotificationId = globalNotificationId; }

	QString msgId() const { return m_msgId; }
	void setMsgId(const QString &msgId) { m_msgId = msgId; }

	QString createTime() const { return m_createTime; }
	void setCreateTime(const QString &createTime) { m_createTime = createTime; }

	bool send() const { return m_send; }
	void setSend(bool send) { m_send = send; }

	QVariantMap toDBMap() const;
	void fromDBMap(const QVariantMap &vm);

	QVariantMap toJSData() const;

	QString bodyText() const;

private:
	quint64 m_innerId;
	QString m_id;
	int     m_type;
	QString m_content;
	QString m_userId;
	QString m_globalNotificationId;
	QString m_msgId;
	QString m_createTime;
	bool    m_send;
};

class GlobalNotificationImageText
{
public:
	GlobalNotificationImageText() {}
	~GlobalNotificationImageText() {}

	void setId(const QString &id) { m_id = id; }
	QString id() const { return m_id; }

	void setKey(const QString &key) { m_key = key; }
	QString key() const { return m_key; }

	void setUrl(const QString &url) { m_url = url; }
	QString url() const { return m_url; }

	void setTitle(const QString &title) { m_title = title; }
	QString title() const { return m_title; }

	void setAuthor(const QString &author) { m_author = author; }
	QString author() const { return m_author; }

	void setSummary(const QString &summary) { m_summary = summary; }
	QString summary() const { return m_summary; }

	void setBody(const QString &body) { m_body = body; }
	QString body() const { return m_body; }

	void setGlobalNotificationId(const QString &globalNotificationId) { m_globalNotificationId = globalNotificationId; }
	QString globalNotificationId() const { return m_globalNotificationId; }

	void setPicUrl(const QString &picUrl) { m_picUrl = picUrl; }
	QString picUrl() const { return m_picUrl; }

	bool isValid() const;

	void parse(const QVariantMap &vm);

private:
	QString m_id;
	QString m_key;
	QString m_url;
	QString m_title;
	QString m_author;
	QString m_summary;
	QString m_body;
	QString m_globalNotificationId;
	QString m_picUrl;
};

class GlobalNotificationImage
{
public:
	GlobalNotificationImage() {}
	~GlobalNotificationImage() {}

	void setName(const QString &name) { m_name = name; }
	QString name() const { return m_name; }

	void setKey(const QString &key) { m_key = key; }
	QString key() const { return m_key; }

	void setId(const QString &id) { m_id = id; }
	QString id() const { return m_id; }

	void setUrl(const QString &url) { m_url = url; }
	QString url() const { return m_url; }

	void setGlobalNotificationId(const QString &globalNotificationId) { m_globalNotificationId = globalNotificationId; }
	QString globalNotificationId() const { return m_globalNotificationId; }

	bool isValid() const;

	void parse(const QVariantMap &vm);

private:
	QString m_name;
	QString m_key;
	QString m_id;
	QString m_url;
	QString m_globalNotificationId;
};

typedef GlobalNotificationImage GlobalNotificationAttach;

#endif // GLOBALNOTIFICATIONMSG_H
