#include "globalnotificationmsgwebpage.h"
#include <QLabel>
#include "PmApp.h"
#include "ModelManager.h"
#include "util/MaskUtil.h"
#include <QWebSettings>
#include <QMouseEvent>
#include "Account.h"
#include "logger/logger.h"
#include <QDebug>

static const char *kUserIdName     = "user_id";
static const char *kWidthName      = "width";
static const char *kHeightName     = "height";

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GlobalNotificationMsgWebPage
GlobalNotificationMsgWebPage::GlobalNotificationMsgWebPage(QWidget *parent)
: CWebPage(parent)
{
	QWebSettings* pWebSettings = this->settings();
	pWebSettings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
	pWebSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
}

GlobalNotificationMsgWebPage::~GlobalNotificationMsgWebPage()
{
}

void GlobalNotificationMsgWebPage::onAvatarChanged(const QString &uid)
{
	foreach (GlobalNotificationMsgAvatarWebObject *obj, m_avatars)
	{
		if (obj->uid() == uid)
		{
			setAvatar(obj, uid);
		}
	}
}

void GlobalNotificationMsgWebPage::onGlobalNotificationAvatarChanged(const QString &globalNotificationId)
{
	foreach (GlobalNotificationMsgAvatarWebObject *obj, m_globalNotificationAvatars)
	{
		if (obj->uid() == globalNotificationId)
		{
			setGlobalNotificationAvatar(obj, globalNotificationId);
		}
	}
}

bool GlobalNotificationMsgWebPage::shouldInterruptJavaScript()
{
	qPmApp->getLogger()->debug(QString::fromLatin1("globalnotification web page rose interrupt javescript error"));
	qDebug() << Q_FUNC_INFO << "rose interrupt javescript error";
	return false;
}

QObject *GlobalNotificationMsgWebPage::createPlugin(const QString &classid, 
											  const QUrl &url, 
											  const QStringList &paramNames, 
											  const QStringList &paramValues)
{
	Q_UNUSED(url);

	GlobalNotificationMsgAvatarWebObject *obj = 0;
	if (classid == "user_avatar")
	{
		int index = paramNames.indexOf(kUserIdName);
		if (index != -1)
		{
			QString uid = paramValues[index];
			obj = new GlobalNotificationMsgAvatarWebObject(uid, view());
			connect(obj, SIGNAL(avatarClicked(QString)), this, SIGNAL(avatarClicked(QString)));
			connect(obj, SIGNAL(destroyed(QObject *)), this, SLOT(onAvatarObjectDestroyed(QObject *)));

			index = paramNames.indexOf(kWidthName);
			int width = paramValues[index].toInt();

			index = paramNames.indexOf(kHeightName);
			int height = paramValues[index].toInt();

			obj->setFixedSize(QSize(width, height));
			setAvatar(obj, uid);

			m_avatars.append(obj);
		}
	}
	else if (classid == "globalnotification_avatar")
	{
		int index = paramNames.indexOf(kUserIdName);
		if (index != -1)
		{
			QString uid = paramValues[index];
			obj = new GlobalNotificationMsgAvatarWebObject(uid, view());
			connect(obj, SIGNAL(avatarClicked(QString)), this, SIGNAL(globalNotificationAvatarClicked(QString)));
			connect(obj, SIGNAL(destroyed(QObject *)), this, SLOT(onGlobalNotificationAvatarObjectDestroyed(QObject *)));

			index = paramNames.indexOf(kWidthName);
			int width = paramValues[index].toInt();

			index = paramNames.indexOf(kHeightName);
			int height = paramValues[index].toInt();

			obj->setFixedSize(QSize(width, height));
			setGlobalNotificationAvatar(obj, uid);

			m_globalNotificationAvatars.append(obj);
		}
	}

	return obj;
}

void GlobalNotificationMsgWebPage::onAvatarObjectDestroyed(QObject *obj)
{
	GlobalNotificationMsgAvatarWebObject *avatarObj = static_cast<GlobalNotificationMsgAvatarWebObject *>(obj);
	if (avatarObj)
	{
		int index = m_avatars.indexOf(avatarObj);
		if (index != -1)
		{
			m_avatars.removeAt(index);
		}
	}
}

void GlobalNotificationMsgWebPage::onGlobalNotificationAvatarObjectDestroyed(QObject *obj)
{
	GlobalNotificationMsgAvatarWebObject *avatarObj = static_cast<GlobalNotificationMsgAvatarWebObject *>(obj);
	if (avatarObj)
	{
		int index = m_globalNotificationAvatars.indexOf(avatarObj);
		if (index != -1)
		{
			m_globalNotificationAvatars.removeAt(index);
		}
	}
}

void GlobalNotificationMsgWebPage::setAvatar(GlobalNotificationMsgAvatarWebObject *obj, const QString &uid)
{
	QSize size = obj->size();
	QPixmap avatar = qPmApp->getModelManager()->getUserAvatar(uid);
	avatar = avatar.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 3;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, size);
	avatar.setMask(mask);
	obj->setPixmap(avatar);
}

void GlobalNotificationMsgWebPage::setGlobalNotificationAvatar(GlobalNotificationMsgAvatarWebObject *obj, const QString &globalNotificationId)
{
	QSize size = obj->size();
	QPixmap avatar = qPmApp->getModelManager()->globalNotificationLogo(globalNotificationId);
	avatar = avatar.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 3;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, size);
	avatar.setMask(mask);
	obj->setPixmap(avatar);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GlobalNotificationMsgAvatarWebObject
GlobalNotificationMsgAvatarWebObject::GlobalNotificationMsgAvatarWebObject(const QString &uid, QWidget *parent /*= 0*/)
: QLabel(parent), m_uid(uid), m_pressed(false) 
{
	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);
	setCursor(Qt::PointingHandCursor);
}

void GlobalNotificationMsgAvatarWebObject::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton)
		m_pressed = true;
}

void GlobalNotificationMsgAvatarWebObject::mouseReleaseEvent(QMouseEvent *ev)
{
	Q_UNUSED(ev);
	if (m_pressed)
	{
		emit avatarClicked(m_uid);
	}
	m_pressed = false;
}
