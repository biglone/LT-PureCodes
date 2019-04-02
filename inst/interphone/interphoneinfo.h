#ifndef INTERPHONEINFO_H
#define INTERPHONEINFO_H

#include "bean/bean.h"
#include <QStringList>

class InterphoneInfo
{
public:
	InterphoneInfo();
	~InterphoneInfo();

	QString id() const;
	void setId(const QString &id);

	bean::MessageType attachType() const;
	void setAttachType(bean::MessageType type);

	QString attachId() const;
	void setAttachId(const QString &id);

	QString speakerId() const;
	void setSpeakerId(const QString &id);

	int memberCount() const;
	void setMemberCount(int count);

	QStringList memberIds() const;
	void setMemberIds(const QStringList &ids);

private:
	QString     m_id;
	bean::MessageType m_attachType;
	QString     m_attachId;
	QString     m_speakerId;
	int         m_memberCount;
	QStringList m_memberIds;
};

#endif // INTERPHONEINFO_H
