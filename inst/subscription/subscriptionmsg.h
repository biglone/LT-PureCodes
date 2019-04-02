#ifndef SUBSCRIPTIONMSG_H
#define SUBSCRIPTIONMSG_H

#include <QString>
#include <QVariantMap>

class SubscriptionMsg
{
public:
	static const int kTypeText      = 1;
	static const int kTypeImageText = 2;
	static const int kTypeImage     = 3;
	static const int kTypeAttach    = 4;

public:
	SubscriptionMsg();
	~SubscriptionMsg();

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

	QString subscriptionId() const { return m_subscriptionId; }
	void setSubscriptionId(const QString &subscriptionId) { m_subscriptionId = subscriptionId; }

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
	QString m_subscriptionId;
	QString m_msgId;
	QString m_createTime;
	bool    m_send;
};

class SubscriptionImageText
{
public:
	SubscriptionImageText() {}
	~SubscriptionImageText() {}

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

	void setSubscriptionId(const QString &subscriptionId) { m_subscriptionId = subscriptionId; }
	QString subscriptionId() const { return m_subscriptionId; }

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
	QString m_subscriptionId;
	QString m_picUrl;
};

class SubscriptionImage
{
public:
	SubscriptionImage() {}
	~SubscriptionImage() {}

	void setName(const QString &name) { m_name = name; }
	QString name() const { return m_name; }

	void setKey(const QString &key) { m_key = key; }
	QString key() const { return m_key; }

	void setId(const QString &id) { m_id = id; }
	QString id() const { return m_id; }

	void setUrl(const QString &url) { m_url = url; }
	QString url() const { return m_url; }

	void setSubscriptionId(const QString &subscriptionId) { m_subscriptionId = subscriptionId; }
	QString subscriptionId() const { return m_subscriptionId; }

	bool isValid() const;

	void parse(const QVariantMap &vm);

private:
	QString m_name;
	QString m_key;
	QString m_id;
	QString m_url;
	QString m_subscriptionId;
};

typedef SubscriptionImage SubscriptionAttach;

#endif // SUBSCRIPTIONMSG_H
