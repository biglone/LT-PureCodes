#include "subscriptionmsgwebpage.h"
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
// MEMBER FUNCTIONS OF CLASS SubscriptionMsgWebPage
SubscriptionMsgWebPage::SubscriptionMsgWebPage(QWidget *parent)
: CWebPage(parent)
{
	QWebSettings* pWebSettings = this->settings();
	pWebSettings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
	pWebSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
}

SubscriptionMsgWebPage::~SubscriptionMsgWebPage()
{
}

void SubscriptionMsgWebPage::onAvatarChanged(const QString &uid)
{
	foreach (SubscriptionMsgAvatarWebObject *obj, m_avatars)
	{
		if (obj->uid() == uid)
		{
			setAvatar(obj, uid);
		}
	}
}

void SubscriptionMsgWebPage::onSubscriptionAvatarChanged(const QString &subscriptionId)
{
	foreach (SubscriptionMsgAvatarWebObject *obj, m_subscriptionAvatars)
	{
		if (obj->uid() == subscriptionId)
		{
			setSubscriptionAvatar(obj, subscriptionId);
		}
	}
}

bool SubscriptionMsgWebPage::shouldInterruptJavaScript()
{
	qPmApp->getLogger()->debug(QString::fromLatin1("subscription web page rose interrupt javescript error"));
	qDebug() << Q_FUNC_INFO << "rose interrupt javescript error";
	return false;
}

QObject *SubscriptionMsgWebPage::createPlugin(const QString &classid, 
											  const QUrl &url, 
											  const QStringList &paramNames, 
											  const QStringList &paramValues)
{
	Q_UNUSED(url);

	SubscriptionMsgAvatarWebObject *obj = 0;
	if (classid == "user_avatar")
	{
		int index = paramNames.indexOf(kUserIdName);
		if (index != -1)
		{
			QString uid = paramValues[index];
			obj = new SubscriptionMsgAvatarWebObject(uid, view());
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
	else if (classid == "subscription_avatar")
	{
		int index = paramNames.indexOf(kUserIdName);
		if (index != -1)
		{
			QString uid = paramValues[index];
			obj = new SubscriptionMsgAvatarWebObject(uid, view());
			connect(obj, SIGNAL(avatarClicked(QString)), this, SIGNAL(subscriptionAvatarClicked(QString)));
			connect(obj, SIGNAL(destroyed(QObject *)), this, SLOT(onSubscriptionAvatarObjectDestroyed(QObject *)));

			index = paramNames.indexOf(kWidthName);
			int width = paramValues[index].toInt();

			index = paramNames.indexOf(kHeightName);
			int height = paramValues[index].toInt();

			obj->setFixedSize(QSize(width, height));
			setSubscriptionAvatar(obj, uid);

			m_subscriptionAvatars.append(obj);
		}
	}

	return obj;
}

void SubscriptionMsgWebPage::onAvatarObjectDestroyed(QObject *obj)
{
	SubscriptionMsgAvatarWebObject *avatarObj = static_cast<SubscriptionMsgAvatarWebObject *>(obj);
	if (avatarObj)
	{
		int index = m_avatars.indexOf(avatarObj);
		if (index != -1)
		{
			m_avatars.removeAt(index);
		}
	}
}

void SubscriptionMsgWebPage::onSubscriptionAvatarObjectDestroyed(QObject *obj)
{
	SubscriptionMsgAvatarWebObject *avatarObj = static_cast<SubscriptionMsgAvatarWebObject *>(obj);
	if (avatarObj)
	{
		int index = m_subscriptionAvatars.indexOf(avatarObj);
		if (index != -1)
		{
			m_subscriptionAvatars.removeAt(index);
		}
	}
}

void SubscriptionMsgWebPage::setAvatar(SubscriptionMsgAvatarWebObject *obj, const QString &uid)
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

void SubscriptionMsgWebPage::setSubscriptionAvatar(SubscriptionMsgAvatarWebObject *obj, const QString &subscriptionId)
{
	QSize size = obj->size();
	QPixmap avatar = qPmApp->getModelManager()->subscriptionLogo(subscriptionId);
	avatar = avatar.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 3;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, size);
	avatar.setMask(mask);
	obj->setPixmap(avatar);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SubscriptionMsgAvatarWebObject
SubscriptionMsgAvatarWebObject::SubscriptionMsgAvatarWebObject(const QString &uid, QWidget *parent /*= 0*/)
: QLabel(parent), m_uid(uid), m_pressed(false) 
{
	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);
	setCursor(Qt::PointingHandCursor);
}

void SubscriptionMsgAvatarWebObject::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton)
		m_pressed = true;
}

void SubscriptionMsgAvatarWebObject::mouseReleaseEvent(QMouseEvent *ev)
{
	Q_UNUSED(ev);
	if (m_pressed)
	{
		emit avatarClicked(m_uid);
	}
	m_pressed = false;
}
