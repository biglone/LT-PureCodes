#include "globalnotificationlastmsgmodel.h"
#include "GlobalNotificationDB.h"
#include "globalnotificationmsg.h"

static const int kRoleId        = Qt::UserRole + 1;
static const int kRoleName      = Qt::UserRole + 2;
static const int kRoleBody      = Qt::UserRole + 3;
static const int kRoleTime      = Qt::UserRole + 4;
static const int kRoleInnerId   = Qt::UserRole + 5;
static const int kRoleSend      = Qt::UserRole + 6;

GlobalNotificationLastMsgModel::GlobalNotificationLastMsgModel(QObject *parent)
	: QStandardItemModel(parent)
{
	setSortRole(kRoleTime);
}

GlobalNotificationLastMsgModel::~GlobalNotificationLastMsgModel()
{

}

void GlobalNotificationLastMsgModel::readFromDB()
{
	if (m_globalNotificationDB.isNull())
	{
		m_globalNotificationDB.reset(new DB::GlobalNotificationDB("GlobalNotificationLastMsgModel"));
	}

	QList<LastMsgItem> lastMsgs = m_globalNotificationDB->lastMsgs();
	setLastMsgs(lastMsgs);
}

void GlobalNotificationLastMsgModel::release()
{
	clear();
	m_items.clear();
	m_globalNotificationDB.reset(0);
}

void GlobalNotificationLastMsgModel::addLastMsg(const GlobalNotificationMsg &msg, const QString &name)
{
	if (!m_globalNotificationDB)
		return;

	if (msg.id().isEmpty())
		return;

	LastMsgItem lastMsg;
	lastMsg.subId = msg.globalNotificationId();
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
	m_globalNotificationDB->addLastMsg(lastMsg);

	emit globalNotificationLastMsgChanged();
}

void GlobalNotificationLastMsgModel::removeLastMsg(const QString &globalNotificationId)
{
	if (globalNotificationId.isEmpty())
		return;

	if (!m_items.contains(globalNotificationId))
		return;

	QStandardItem *item = m_items[globalNotificationId];
	takeRow(item->row());
	delete item;
	item = 0;
	m_items.remove(globalNotificationId);

	// remove from db
	m_globalNotificationDB->removeLastMsg(globalNotificationId);

	emit globalNotificationLastMsgChanged();
}

GlobalNotificationLastMsgModel::LastMsgItem GlobalNotificationLastMsgModel::latestMsg() const
{
	LastMsgItem lastMsg;
	if (rowCount() > 0)
	{
		QStandardItem *item = this->item(0);
		lastMsg = modelItem2LastMsg(*item);
	}
	return lastMsg;
}

QStringList GlobalNotificationLastMsgModel::allGlobalNotificationIds() const
{
	return m_items.keys();
}

QStandardItem *GlobalNotificationLastMsgModel::lastMsg2ModelItem(const LastMsgItem &lastMsg)
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

GlobalNotificationLastMsgModel::LastMsgItem GlobalNotificationLastMsgModel::modelItem2LastMsg(const QStandardItem &item)
{
	GlobalNotificationLastMsgModel::LastMsgItem lastMsg;
	lastMsg.subId = item.data(kRoleId).toString();
	lastMsg.subName = item.data(kRoleName).toString();
	lastMsg.lastBody = item.data(kRoleBody).toString();
	lastMsg.lastTime = item.data(kRoleTime).toString();
	lastMsg.innerId = item.data(kRoleInnerId).toString();
	lastMsg.send = (item.data(kRoleSend).toInt() == 1 ? true : false);
	return lastMsg;
}

void GlobalNotificationLastMsgModel::setLastMsgs(const QList<LastMsgItem> &lastMsgs)
{
	clear();
	foreach (LastMsgItem lastMsg, lastMsgs)
	{
		QStandardItem *item = lastMsg2ModelItem(lastMsg);
		appendRow(item);
		m_items.insert(lastMsg.subId, item);
	}

	sort(0, Qt::DescendingOrder);

	emit globalNotificationLastMsgChanged();
}