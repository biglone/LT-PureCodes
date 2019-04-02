#include "chatwebpage.h"
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
static const char *kChatTypeName   = "chat_type";
static const char *kOtherIdName    = "other_id";
static const char *kGroupIdName    = "group_id";

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS ChatWebPage
ChatWebPage::ChatWebPage(QWidget *parent)
	: CWebPage(parent)
{
	QWebSettings* pWebSettings = this->settings();
	pWebSettings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
	pWebSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
}

ChatWebPage::~ChatWebPage()
{
}

void ChatWebPage::onAvatarChanged(const QString &uid)
{
	foreach (AvatarWebObject *obj, m_avatars)
	{
		if (obj->uid() == uid)
		{
			setAvatar(obj, uid, obj->otherId(), obj->chatType());
		}
	}
}

bool ChatWebPage::shouldInterruptJavaScript()
{
	qPmApp->getLogger()->debug(QString::fromLatin1("chat web page rose interrupt javescript error"));
	qDebug() << Q_FUNC_INFO << "rose interrupt javescript error";
	return false;
}

QObject *ChatWebPage::createPlugin(const QString &classid, 
								   const QUrl &url, 
								   const QStringList &paramNames, 
								   const QStringList &paramValues)
{
	Q_UNUSED(url);

	AvatarWebObject *obj = 0;
	if (classid == "user_avatar")
	{
		int index = paramNames.indexOf(kUserIdName);
		if (index != -1)
		{
			QString uid = paramValues[index];
			obj = new AvatarWebObject(uid, view());
			connect(obj, SIGNAL(avatarClicked(QString)), this, SIGNAL(avatarClicked(QString)));
			connect(obj, SIGNAL(destroyed(QObject *)), this, SLOT(onAvatarObjectDestroyed(QObject *)));
			connect(obj, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
			
			index = paramNames.indexOf(kWidthName);
			int width = paramValues[index].toInt();

			index = paramNames.indexOf(kHeightName);
			int height = paramValues[index].toInt();

			index = paramNames.indexOf(kChatTypeName);
			QString chatType = paramValues[index];

			index = paramNames.indexOf(kOtherIdName);
			QString otherId = paramValues[index];

			index = paramNames.indexOf(kGroupIdName);
			QString groupId = paramValues[index];

			obj->setFixedSize(QSize(width, height));
			obj->setChatType(chatType);
			obj->setOtherId(otherId);
			obj->setGroupId(groupId);

			setAvatar(obj, uid, otherId, chatType);

			m_avatars.append(obj);
		}
	}

	return obj;
}

void ChatWebPage::onAvatarObjectDestroyed(QObject *obj)
{
	AvatarWebObject *avatarObj = static_cast<AvatarWebObject *>(obj);
	if (avatarObj)
	{
		int index = m_avatars.indexOf(avatarObj);
		if (index != -1)
		{
			m_avatars.removeAt(index);
		}
	}
}

void ChatWebPage::onCustomContextMenuRequested(const QPoint &pos)
{
	AvatarWebObject *avatarObj = qobject_cast<AvatarWebObject *>(sender());
	if (!avatarObj)
		return;

	emit avatarContextMenu(pos, avatarObj->chatType(), avatarObj->groupId(), avatarObj->uid());
}

void ChatWebPage::setAvatar(AvatarWebObject *obj, const QString &uid, const QString &otherId, const QString &chatType)
{
	QSize size = obj->size();
	QPixmap avatar;
	if (uid == Account::instance()->id() && 
		otherId == Account::instance()->phoneFullId() && 
		chatType == bean::kszChat)
	{
		avatar = QPixmap(":/images/mycomputer.png");
		obj->setClickable(false);
	}
	else if (uid == Account::instance()->phoneFullId() &&
		chatType == bean::kszChat)
	{
		avatar = QPixmap(":/images/myphone.png");
		obj->setClickable(false);
	}
	else
	{
		avatar = qPmApp->getModelManager()->getUserAvatar(uid);
		obj->setClickable(true);
	}
	avatar = avatar.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 3;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, size);
	avatar.setMask(mask);
	obj->setPixmap(avatar);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AvatarWebObject
AvatarWebObject::AvatarWebObject(const QString &uid, QWidget *parent /*= 0*/)
: QLabel(parent), m_uid(uid), m_clickable(true), m_pressed(false) 
{
	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);
	setCursor(Qt::PointingHandCursor);
	setContextMenuPolicy(Qt::CustomContextMenu);
}

void AvatarWebObject::mousePressEvent(QMouseEvent *ev)
{
	if (ev->button() == Qt::LeftButton)
		m_pressed = true;
}

void AvatarWebObject::mouseReleaseEvent(QMouseEvent *ev)
{
	Q_UNUSED(ev);
	if (m_pressed)
	{
		if (m_clickable)
		{
			emit avatarClicked(m_uid);
		}
	}
	m_pressed = false;
}

void AvatarWebObject::setClickable(bool clickable)
{
	m_clickable = clickable;
	if (m_clickable)
		setCursor(Qt::PointingHandCursor);
	else
		setCursor(Qt::ArrowCursor);
}
