#include "unreadmsgmodel.h"
#include "unreadmsgitem.h"
#include "Constants.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "unreadmessagesortfiltermodel.h"

UnreadMsgModel::UnreadMsgModel(QObject *parent)
	: QStandardItemModel(parent)
{
	m_filterModel = new UnreadMessageSortFilterModel(this);
	m_filterModel->setSourceModel(this);
}

UnreadMsgModel::~UnreadMsgModel()
{

}

UnreadMessageSortFilterModel *UnreadMsgModel::filterModel() const
{
	return m_filterModel;
}

void UnreadMsgModel::insertMsg(const QString &id, 
							   bean::MessageType msgType, 
							   const bean::MessageBody &msg, 
							   bool msgAtFirst /*= false*/,
							   bool ignore /*= false*/)
{
	QString msgItemKey = getMsgItemKey(id, msgType);
	if (m_unreadMsgItems.contains(msgItemKey))
	{
		// insert message to the existing item
		UnreadMsgItem *msgItem = m_unreadMsgItems[msgItemKey];
		if (!msgAtFirst)
			msgItem->appendMsg(msg);
		else
			msgItem->insertMsgToTop(msg);
		msgItem->setIgnoreBefore(ignore);

		// if this message is not the top one, move to top and notify last message changed
		if (!ignore)
		{
			UnreadMsgItem *topMsgItem = (UnreadMsgItem *)(this->item(0));
			if (topMsgItem != msgItem)
			{
				this->takeRow(msgItem->row());
				this->insertRow(0, msgItem);
			}

			// notify last message changed
			emit lastMsgChanged(msgItem->id(), msgItem->msgType());
		}
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		QString name;
		QIcon icon;
		if (msgType == bean::Message_GroupChat)
		{
			name = modelManager->groupName(id);
			icon = modelManager->getGroupLogo(id);
		}
		else if (msgType == bean::Message_DiscussChat)
		{
			name = modelManager->discussName(id);
			icon = modelManager->discussLogo(id);
		}
		else
		{
			name = modelManager->userName(id);
			icon = modelManager->getUserAvatar(id);
		}

		// create a new message item
		UnreadMsgItem *msgItem = new UnreadMsgItem();
		msgItem->setId(id);
		msgItem->setMsgType(msgType);
		msgItem->setName(name);
		msgItem->setIcon(icon);
		if (!msgAtFirst)
			msgItem->appendMsg(msg);
		else
			msgItem->insertMsgToTop(msg);
		msgItem->setIgnoreBefore(ignore);

		// add this item to model
		if (!ignore)
		{
			this->insertRow(0, msgItem);
			m_unreadMsgItems[msgItemKey] = msgItem;

			// notify last message changed
			emit lastMsgChanged(msgItem->id(), msgItem->msgType());
		}
		else
		{
			this->appendRow(msgItem);
			m_unreadMsgItems[msgItemKey] = msgItem;
		}
	}

	emit unreadItemCountChanged();
}

QList<bean::MessageBody> UnreadMsgModel::takeMsg(const QString &id, bean::MessageType msgType)
{
	QList<bean::MessageBody> msgs;
	QString msgItemKey = getMsgItemKey(id, msgType);
	if (!m_unreadMsgItems.contains(msgItemKey))
		return msgs;

	// take from model and delete it
	UnreadMsgItem *msgItem = m_unreadMsgItems[msgItemKey];
	emit preTakeMsg(msgItem->index());
	this->takeRow(msgItem->row());
	msgs = msgItem->msgs();
	delete msgItem;
	msgItem = 0;

	// clear from map
	m_unreadMsgItems.remove(msgItemKey);

	emit msgToken(id, msgType);

	// if still has msg, notify outside
	if (this->rowCount() > 0)
	{
		UnreadMsgItem *topMsgItem = (UnreadMsgItem *)(this->item(0));
		if (!topMsgItem->isIgnoreBefore())
			emit lastMsgChanged(topMsgItem->id(), topMsgItem->msgType());
		else
			emit lastMsgChanged(QString(), bean::Message_Invalid);
	}
	else
	{
		emit lastMsgChanged(QString(), bean::Message_Invalid);
	}

	emit unreadItemCountChanged();

	return msgs;
}

void UnreadMsgModel::clean()
{
	clear();
	m_unreadMsgItems.clear();
	emit lastMsgChanged(QString(), bean::Message_Invalid);
	emit unreadItemCountChanged();
}

bool UnreadMsgModel::containsMsg(const QString &id, bean::MessageType msgType)
{
	QString msgItemKey = getMsgItemKey(id, msgType);
	return m_unreadMsgItems.contains(msgItemKey);
}

bool UnreadMsgModel::hasUnreadMsg() const
{
	if (m_unreadMsgItems.isEmpty())
		return false;

	UnreadMsgItem *msgItem = (UnreadMsgItem *)this->item(0);
	if (msgItem->isIgnoreBefore())
		return false;

	return true;
}

bool UnreadMsgModel::getTopUnreadMsg(QString &id, bean::MessageType &msgType) const
{
	if (!hasUnreadMsg())
		return false;

	UnreadMsgItem *msgItem = (UnreadMsgItem *)this->item(0);
	id = msgItem->id();
	msgType = msgItem->msgType();
	return true;
}

void UnreadMsgModel::ignoreAll()
{
	emit preIgnoreAll();

	clean();
}

QList<UnreadMsgItem *> UnreadMsgModel::allUnreadMsgs() const
{
	return m_unreadMsgItems.values();
}

QList<QPair<QString, bean::MessageType>> UnreadMsgModel::allUnreadMsgsFrom() const
{
	QList<QPair<QString, bean::MessageType>> froms;
	foreach (QString msgItemKey, m_unreadMsgItems.keys())
	{
		QPair<QString, bean::MessageType> pair;
		fromMsgItemKey(msgItemKey, pair.first, pair.second);
		froms.append(pair);
	}
	return froms;
}

UnreadMsgItem * UnreadMsgModel::peekUnreadMsg(const QString &id, bean::MessageType msgType) const
{
	QString msgItemKey = getMsgItemKey(id, msgType);
	UnreadMsgItem *item = 0;
	if (m_unreadMsgItems.contains(msgItemKey))
	{
		item = m_unreadMsgItems[msgItemKey];
	}
	return item;
}

QString UnreadMsgModel::getMsgItemKey(const QString &rsUid, bean::MessageType type) const
{
	return QString(MAP_DIALOG_KEY_FORMAT).arg(int(type)).arg(MAP_KEY_SEPARATION).arg(rsUid);
}

bool UnreadMsgModel::fromMsgItemKey(const QString &msgItemKey, QString &rsUid, bean::MessageType &type) const
{
	bool ok = false;

	do 
	{
		if (msgItemKey.isEmpty())
			break;

		QStringList parts = msgItemKey.split(MAP_KEY_SEPARATION);
		if (parts.count() != 2)
			break;

		type = (bean::MessageType)parts[0].toInt();
		rsUid = parts[1];

		ok = true;
	} while (0);

	return ok;
}