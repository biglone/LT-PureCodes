#include "changenoticemgr.h"
#include <QMetaType>
#include "pmclient/PmClient.h"
#include <QDebug>
#include <QString>
#include "protocol/ProtocolType.h"
#include "addfriendmanager.h"
#include <QByteArray>
#include "Account.h"

ChangeNoticeMgr::ChangeNoticeMgr(QObject *parent)
	: QObject(parent), m_nHandleId(-1)
{
	qRegisterMetaType< QList<ChangeNoticeMgr::Event> >("QList<ChangeNoticeMgr::Event>");
}

ChangeNoticeMgr::~ChangeNoticeMgr()
{

}

void ChangeNoticeMgr::postGroupChangeNotice(const QString &groupId, const QString &changeType)
{
	if (changeType != "add" && changeType != "change" && changeType != "delete")
		return;

	QString base64Id = QString::fromLatin1(groupId.toUtf8().toBase64().constData());
	QString param = QString("%1:%2").arg(base64Id).arg(changeType);
	emit groupChangeNotice(param);
}

void ChangeNoticeMgr::postDiscussChangeNotice(const QString &discussId, const QString &changeType)
{
	// kick 是自己创建的，服务器不会向客户端推送kick，kick为了区别讨论组删除和被踢出讨论组
	if (changeType != "add" && changeType != "change" && changeType != "delete" && changeType != "kick")
		return;

	QString base64Id = QString::fromLatin1(discussId.toUtf8().toBase64().constData());
	QString param = QString("%1:%2").arg(base64Id).arg(changeType);
	emit discussChangeNotice(param);
}

bool ChangeNoticeMgr::initObject()
{
	m_nHandleId = PmClient::instance()->insertNotificationHandler(this);
	if (m_nHandleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qWarning() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void ChangeNoticeMgr::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* ChangeNoticeMgr::instance()
{
	return this;
}

QList<int> ChangeNoticeMgr::types() const
{
	return QList<int>() << protocol::CHANGENOTICE;
}

int ChangeNoticeMgr::handledId() const
{
	return m_nHandleId;
}

bool ChangeNoticeMgr::onNotication(int handleId, protocol::SpecificNotification* sn)
{
	if (m_nHandleId != handleId)
		return false;

	protocol::ChangeNoticeNotification *pNotice = static_cast<protocol::ChangeNoticeNotification *>(sn);

	if (pNotice)
	{
		QList<ChangeNoticeMgr::Event> noticeEvents;
		std::vector<protocol::ChangeNoticeNotification::Event> events = pNotice->getEvents();
		for (int i = 0; i < (int)events.size(); i++)
		{
			protocol::ChangeNoticeNotification::Event event = events[i];
			ChangeNoticeMgr::Event noticeEvent;
			noticeEvent.name = QString::fromUtf8(event.name.c_str());
			//noticeEvent.eid = QString::fromUtf8(event.eid.c_str());
			noticeEvent.param.v = QString::fromUtf8(event.param.v.c_str());
			noticeEvents.append(noticeEvent);
		}

		QMetaObject::invokeMethod(this, "processChangeNotice", Qt::QueuedConnection, Q_ARG(QList<ChangeNoticeMgr::Event>, noticeEvents));
	}

	return true;
}

void ChangeNoticeMgr::processChangeNotice(const QList<ChangeNoticeMgr::Event> &changeEvents)
{
	foreach (ChangeNoticeMgr::Event changeEvent, changeEvents)
	{
		QString param = changeEvent.param.v;
		if (0 == QString::compare(changeEvent.name, "group_change", Qt::CaseInsensitive))
		{
			emit groupChangeNotice(param);
		}
		else if (0 == QString::compare(changeEvent.name, "discuss_change", Qt::CaseInsensitive))
		{
			emit discussChangeNotice(param);
		}
		else if (0 == QString::compare(changeEvent.name, "roster_request", Qt::CaseInsensitive))
		{
			emit rosterAddNotice(AddFriendManager::Request, param);
		}
		else if (0 == QString::compare(changeEvent.name, "roster_accept", Qt::CaseInsensitive))
		{
			emit rosterAddNotice(AddFriendManager::Accept, param);
		}
		else if (0 == QString::compare(changeEvent.name, "roster_refuse", Qt::CaseInsensitive))
		{
			emit rosterAddNotice(AddFriendManager::Refuse, param);
		}
		else if (0 == QString::compare(changeEvent.name, "roster_responded", Qt::CaseInsensitive))
		{
			emit rosterAddResponded(param);
		}
		else if (0 == QString::compare(changeEvent.name, "subscription_text", Qt::CaseInsensitive) ||
			     0 == QString::compare(changeEvent.name, "subscription_article_tip", Qt::CaseInsensitive) ||
				 0 == QString::compare(changeEvent.name, "subscription_img_tip", Qt::CaseInsensitive) ||
				 0 == QString::compare(changeEvent.name, "subscription_file_tip", Qt::CaseInsensitive))
		{
			emit hasSubscriptionMsg();
		}
		else if (0 == QString::compare(changeEvent.name, "subscription_subscribed", Qt::CaseInsensitive))
		{
			emit subscriptionSubscribed(param);
		}
		else if (0 == QString::compare(changeEvent.name, "subscription_unsubscribed", Qt::CaseInsensitive))
		{
			emit subscriptionUnsubscribed(param);
		}
		else if (0 == QString::compare(changeEvent.name, "roster_delete", Qt::CaseInsensitive))
		{
			processDeleteFriend(param);
		}
		else if (0 == QString::compare(changeEvent.name, "roster_delete_responded", Qt::CaseInsensitive))
		{
			processDeleteFriend(param);
		}
		else if (0 == QString::compare(changeEvent.name, "secret_ack", Qt::CaseInsensitive))
		{
			processSecretAck(param);
		}
		else if (0 == QString::compare(changeEvent.name, "secret_acked", Qt::CaseInsensitive))
		{
			processSecretAcked(param);
		}
		else if (0 == QString::compare(changeEvent.name, "config_change", Qt::CaseInsensitive))
		{
			emit configChanged(param);
		}
		else if (0 == QString::compare(changeEvent.name, "passwd_change", Qt::CaseInsensitive))
		{
			emit passwdModified();
		}
		else if (0 == QString::compare(changeEvent.name, "user_deleted", Qt::CaseInsensitive))
		{
			// 用户在后台被删除了
			processDeleteUser(param);
		}
		else if (0 == QString::compare(changeEvent.name, "user_frozen", Qt::CaseInsensitive))
		{
			// 用户在后台被冻结了
			processFreezeUser(param);
		}
	}
}

void ChangeNoticeMgr::processDeleteFriend(const QString &param)
{
	int index = param.indexOf(":");
	if (index == -1)
		return;

	QString base64FromId = param.left(index);
	QString fromId = QString::fromUtf8(QByteArray::fromBase64(base64FromId.toLatin1()));
	QString base64ToId = param.right(index+1);
	QString toId = QString::fromUtf8(QByteArray::fromBase64(base64ToId.toLatin1()));
	QString selfId = Account::instance()->id();
	QString otherId = fromId;
	if (otherId == selfId)
		otherId = toId;
	emit deleteFriend(otherId);
}

void ChangeNoticeMgr::processSecretAck(const QString &param)
{
	int index = param.indexOf(":");
	if (index == -1)
		return;

	QString stamp = param.left(index);
	QString fromUid = param.mid(index+1);
	emit secretAckRecved(fromUid, stamp, 1);
}

void ChangeNoticeMgr::processSecretAcked(const QString &param)
{
	int index = param.indexOf(":");
	if (index == -1)
		return;

	QString stamp = param.left(index);
	QString toUid = param.mid(index+1);
	emit secretAcked(toUid, stamp);
}

void ChangeNoticeMgr::processDeleteUser(const QString &param)
{
	if (param.isEmpty())
		return;

	QString deletedId = QString::fromUtf8(QByteArray::fromBase64(param.toLatin1()));
	emit userDeleted(deletedId);
}

void ChangeNoticeMgr::processFreezeUser(const QString &param)
{
	if (param.isEmpty())
		return;

	QString frozenId = QString::fromUtf8(QByteArray::fromBase64(param.toLatin1()));
	emit userFrozen(frozenId);
}