#include "subscriptionlastmsgmodel.h"
#include "SubscriptionDB.h"
#include "subscriptionmsg.h"

static const int kRoleId        = Qt::UserRole + 1;
static const int kRoleName      = Qt::UserRole + 2;
static const int kRoleBody      = Qt::UserRole + 3;
static const int kRoleTime      = Qt::UserRole + 4;
static const int kRoleInnerId   = Qt::UserRole + 5;
static const int kRoleSend      = Qt::UserRole + 6;

SubscriptionLastMsgModel::SubscriptionLastMsgModel(QObject *parent)
	: QStandardItemModel(parent)
{
	setSortRole(kRoleTime);
}

SubscriptionLastMsgModel::~SubscriptionLastMsgModel()
{

}

void SubscriptionLastMsgModel::readFromDB()
{
	if (m_subscriptionDB.isNull())
	{
		m_subscriptionDB.reset(new DB::SubscriptionDB("SubscriptionLastMsgModel"));
	}

	QList<LastMsgItem> lastMsgs = m_subscriptionDB->lastMsgs();
	setLastMsgs(lastMsgs);
}

void SubscriptionLastMsgModel::release()
{
	clear();
	m_items.clear();
	m_subscriptionDB.reset(0);
}

void SubscriptionLastMsgModel::addLastMsg(const SubscriptionMsg &msg, const QString &name)
{
	if (!m_subscriptionDB)
		return;

	if (msg.id().isEmpty())
		return;

	LastMsgItem lastMsg;
	lastMsg.subId = msg.subscriptionId();
	lastMsg.subName = name;
	QString lastBody = msg.bodyText();
	if (msg.send())
	{
		lastBody = QString("%1: %2").arg(tr("I")).arg(msg.bodyText());
	}
	lastMsg.lastBody = lastBody;
	lastMsg.lastTime = msg.createTime();
	lastMsg.innerId = QString::number(msg.innerId());
	lastMsg.send = msg.send();
	if (m_items.contains(lastMsg.subId))
	{
		QStandardItem *origItem = m_items[lastMsg.subId];
		takeRow(origItem->row());
		delete origItem;
		origItem = 0;
		m_items.remove(lastMsg.subId);
	}
	
	// add to model
	QStandardItem *item = lastMsg2ModelItem(lastMsg);
	appendRow(item);
	m_items.insert(lastMsg.subId, item);

	sort(0, Qt::DescendingOrder);

	// save to db
	m_subscriptionDB->addLastMsg(lastMsg);

	emit subscriptionLastMsgChanged();
}

void SubscriptionLastMsgModel::removeLastMsg(const QString &subscriptionId)
{
	if (subscriptionId.isEmpty())
		return;

	if (!m_items.contains(subscriptionId))
		return;

	QStandardItem *item = m_items[subscriptionId];
	takeRow(item->row());
	delete item;
	item = 0;
	m_items.remove(subscriptionId);

	// remove from db
	m_subscriptionDB->removeLastMsg(subscriptionId);

	emit subscriptionLastMsgChanged();
}

SubscriptionLastMsgModel::LastMsgItem SubscriptionLastMsgModel::latestMsg() const
{
	LastMsgItem lastMsg;
	if (rowCount() > 0)
	{
		QStandardItem *item = this->item(0);
		lastMsg = modelItem2LastMsg(*item);
	}
	return lastMsg;
}

QStringList SubscriptionLastMsgModel::allSubscriptionIds() const
{
	return m_items.keys();
}

QStandardItem *SubscriptionLastMsgModel::lastMsg2ModelItem(const LastMsgItem &lastMsg)
{
	QStandardItem *item = new QStandardItem();
	item->setData(lastMsg.subId, kRoleId);
	item->setData(lastMsg.subName, kRoleName);
	item->setData(lastMsg.lastBody, kRoleBody);
	item->setData(lastMsg.lastTime, kRoleTime);
	item->setData(lastMsg.innerId, kRoleInnerId);
	item->setData(lastMsg.send, kRoleSend);
	return item;
}

SubscriptionLastMsgModel::LastMsgItem SubscriptionLastMsgModel::modelItem2LastMsg(const QStandardItem &item)
{
	SubscriptionLastMsgModel::LastMsgItem lastMsg;
	lastMsg.subId = item.data(kRoleId).toString();
	lastMsg.subName = item.data(kRoleName).toString();
	lastMsg.lastBody = item.data(kRoleBody).toString();
	lastMsg.lastTime = item.data(kRoleTime).toString();
	lastMsg.innerId = item.data(kRoleInnerId).toString();
	lastMsg.send = (item.data(kRoleSend).toInt() == 1 ? true : false);
	return lastMsg;
}

void SubscriptionLastMsgModel::setLastMsgs(const QList<LastMsgItem> &lastMsgs)
{
	clear();
	foreach (LastMsgItem lastMsg, lastMsgs)
	{
		QStandardItem *item = lastMsg2ModelItem(lastMsg);
		appendRow(item);
		m_items.insert(lastMsg.subId, item);
	}

	sort(0, Qt::DescendingOrder);

	emit subscriptionLastMsgChanged();
}