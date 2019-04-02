#include "globalnotificationmsg.h"
#include "qt-json/json.h"
#include <QDateTime>
#include "common/datetime.h"

static const char *kFieldInnerId = "innerId";
static const char *kFieldId = "id";
static const char *kFieldMsgId = "msgId";
static const char *kFieldUserId = "userId";
static const char *kFieldGlobalNotificationId = "globalNotificationId";
static const char *kFieldType = "type";
static const char *kFieldContent = "content";
static const char *kFieldCreateTime = "createTime";
static const char *kFieldSend = "send";
static const char *kFieldWidth = "width";
static const char *kFieldHeight = "height";

static const char *kTagKey = "key";
static const char *kTagId = "id";
static const char *kTagUrl = "url";
static const char *kTagGlobalNotificationId = "globalNotificationId";
static const char *kTagPicUrl = "picUrl";
static const char *kTagTitle = "title";
static const char *kTagAuthor = "author";
static const char *kTagSummary = "summary";
static const char *kTagBody = "body";
static const char *kTagName = "name";

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GlobalNotificationMsg
GlobalNotificationMsg::GlobalNotificationMsg()
: m_type(0), m_send(false), m_innerId(0)
{

}

GlobalNotificationMsg::~GlobalNotificationMsg()
{

}

QVariantMap GlobalNotificationMsg::toDBMap() const
{
	QVariantMap vm;
	vm[kFieldId] = m_id;
	vm[kFieldMsgId] = m_msgId;
	vm[kFieldUserId] = m_userId;
	vm[kFieldGlobalNotificationId] = m_globalNotificationId;
	vm[kFieldType] = m_type;
	vm[kFieldContent] = m_content;
	vm[kFieldCreateTime] = m_createTime;
	vm[kFieldSend] = (m_send ? 1 : 0);
	return vm;
}

void GlobalNotificationMsg::fromDBMap(const QVariantMap &vm)
{
	m_innerId = vm[kFieldInnerId].toULongLong();
	m_id = vm[kFieldId].toString();
	m_msgId = vm[kFieldMsgId].toString();
	m_userId = vm[kFieldUserId].toString();
	m_globalNotificationId = vm[kFieldGlobalNotificationId].toString();
	m_type = vm[kFieldType].toInt();
	m_content = vm[kFieldContent].toString();
	m_createTime = vm[kFieldCreateTime].toString();
	m_send = vm[kFieldSend].toInt();
}

QVariantMap GlobalNotificationMsg::toJSData() const
{
	QVariantMap vm = toDBMap();

	if (m_type == kTypeImageText || m_type == kTypeImage || m_type == kTypeAttach)
	{
		vm[kFieldContent] = QtJson::parse(m_content);

		if (m_type == kTypeImage)
		{
			QVariantMap contentVm = vm[kFieldContent].toMap();
			int width = 0;
			int height = 0;
			if (contentVm.contains(kFieldWidth) && contentVm.contains(kFieldHeight))
			{
				width = contentVm[kFieldWidth].toInt();
				height = contentVm[kFieldHeight].toInt();
				if (width > 0 && height > 0)
				{
					const int kMaxWidth = 343;
					if (width > kMaxWidth)
					{
						height = (int)((float)height*((float)kMaxWidth/(float)width));
						width = kMaxWidth;
					}
				}
				else
				{
					width = 0;
					height = 0;
				}
			}

			contentVm[kFieldWidth] = width;
			contentVm[kFieldHeight] = height;
			vm[kFieldContent] = contentVm;
		}
	}

	QString timeString;
	if (!m_createTime.isEmpty())
	{
		QDateTime curDateTime = CDateTime::currentDateTime();
		QDateTime msgDateTime = CDateTime::localDateTimeFromUtcString(m_createTime);
		if (msgDateTime.daysTo(curDateTime) == 0)
			timeString = msgDateTime.toString("hh:mm:ss");
		else
			timeString = msgDateTime.toString("M.d hh:mm:ss");
	}
	vm[kFieldCreateTime] = timeString;

	return vm;
}

QString GlobalNotificationMsg::bodyText() const
{
	QString body;
	if (this->type() == GlobalNotificationMsg::kTypeImageText)
		body = QObject::tr("[Text&Image]");
	else if (this->type() == GlobalNotificationMsg::kTypeImage)
		body = QObject::tr("[Image]");
	else if (this->type() == GlobalNotificationMsg::kTypeAttach)
		body = QObject::tr("[Attach]");
	else
		body = this->content();
	return body;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GlobalNotificationImageText
bool GlobalNotificationImageText::isValid() const
{
	return !m_id.isEmpty();
}

void GlobalNotificationImageText::parse(const QVariantMap &vm)
{
	m_id = QString::number(vm[kTagId].toULongLong());
	m_key = vm[kTagKey].toString();
	m_url = vm[kTagUrl].toString();
	m_title = vm[kTagTitle].toString();
	m_author = vm[kTagAuthor].toString();
	m_summary = vm[kTagSummary].toString();
	m_body = vm[kTagBody].toString();
	m_globalNotificationId = QString::number(vm[kTagGlobalNotificationId].toULongLong());
	m_picUrl = vm[kTagPicUrl].toString();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GlobalNotificationImage
bool GlobalNotificationImage::isValid() const
{
	return !m_id.isEmpty();
}

void GlobalNotificationImage::parse(const QVariantMap &vm)
{
	m_name = vm[kTagName].toString();
	m_key = vm[kTagKey].toString();
	m_id = QString::number(vm[kTagId].toULongLong());
	m_url = vm[kTagUrl].toString();
	m_globalNotificationId = QString::number(vm[kTagGlobalNotificationId].toULongLong());
}
