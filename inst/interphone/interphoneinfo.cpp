#include "interphoneinfo.h"

InterphoneInfo::InterphoneInfo()
{
	m_attachType = bean::Message_Invalid;
	m_memberCount = 0;
}

InterphoneInfo::~InterphoneInfo()
{

}

QString InterphoneInfo::id() const
{
	return m_id;
}

void InterphoneInfo::setId(const QString &id)
{
	m_id = id;
}

bean::MessageType InterphoneInfo::attachType() const
{
	return m_attachType;
}

void InterphoneInfo::setAttachType(bean::MessageType type)
{
	m_attachType = type;
}

QString InterphoneInfo::attachId() const
{
	return m_attachId;
}

void InterphoneInfo::setAttachId(const QString &id)
{
	m_attachId = id;
}

QString InterphoneInfo::speakerId() const
{
	return m_speakerId;
}

void InterphoneInfo::setSpeakerId(const QString &id)
{
	m_speakerId = id;
}

int InterphoneInfo::memberCount() const
{
	return m_memberCount;
}

void InterphoneInfo::setMemberCount(int count)
{
	m_memberCount = count;
}

QStringList InterphoneInfo::memberIds() const
{
	return m_memberIds;
}

void InterphoneInfo::setMemberIds(const QStringList &ids)
{
	m_memberIds = ids;
}
