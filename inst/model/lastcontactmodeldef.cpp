#include "lastcontactmodeldef.h"
#include "ModelManager.h"
#include "db/LastContactDB.h"
#include "Constants.h"
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include "unreadmsgmodel.h"
#include "unreadmsgitem.h"
#include "PmApp.h"
#include "manager/offlinemsgmanager.h"
#include "msgmultisenddlg.h"
#include <QUuid>
#include "subscriptionlastmsgmodel.h"

const char *LastContactModel::TYPE_CHAT         = "chat";
const char *LastContactModel::TYPE_GROUPCHAT    = "groupchat";
const char *LastContactModel::TYPE_DISCUSS      = "discuss";
const char *LastContactModel::TYPE_MULTISEND    = "multisend";

LastContactModel::LastContactModel(ModelManager *parent)
	: QStandardItemModel(parent), m_pModelManager(parent), m_bInit(false)
{
	m_pContactDB.reset(new DB::LastContactDB());

	m_pProxyModel = new QSortFilterProxyModel(this);
	m_pProxyModel->setSourceModel(this);
	m_pProxyModel->setFilterKeyColumn(0);
	m_pProxyModel->setSortRole(LastContactItem::LastContactTimeRole);
	m_pProxyModel->setDynamicSortFilter(true);

	m_writeTimer.setSingleShot(true);
	m_writeTimer.setInterval(1000);
	connect(&m_writeTimer, SIGNAL(timeout()), this, SLOT(commitAll()));
}

LastContactModel::~LastContactModel()
{
}

bool LastContactModel::init()
{
	if (m_bInit)
		return true;

	m_bInit = readData();
	if (!m_bInit)
	{
		release();
	}
	else
	{
		m_pProxyModel->sort(0, Qt::DescendingOrder);
		onUnreadItemCountChanged();
	}

	return m_bInit;
}

void LastContactModel::release()
{
	m_writeTimer.stop();
	commitAll();

	clear();
	m_records.clear();
	m_bInit = false;
}

void LastContactModel::appendMsg(const bean::MessageBody &sMsg)
{
	LastContactItem::LastContactItemType itemType = LastContactItem::LastContactTypeContact;
	switch (sMsg.messageType())
	{
	case bean::Message_Chat:
		itemType = LastContactItem::LastContactTypeContact;
		break;
	case bean::Message_GroupChat:
		itemType = LastContactItem::LastContactTypeGroupMuc;
		break;
	case bean::Message_DiscussChat:
		itemType = LastContactItem::LastContactTypeDiscuss;
		break;
	default:
		qDebug("%s messagetype invalid.", Q_FUNC_INFO);
		return;
	}

	QString sKey = makeKey(itemType, sMsg.to());

	// get the last contact item
	LastContactItem *item = 0;
	if (m_records.contains(sKey))
	{
		item = m_records[sKey];
		if (sMsg.time() < item->lastTime())
		{
			// !!! do not accept the message which is earlier than the current one
			return;
		}
		else if (sMsg.time() == item->lastTime())
		{
			// !!! do not accept the message which is earlier than the current one
			if (!sMsg.isSend() && sMsg.stamp() <= item->timeStamp())
				return;
		}
	}
	else
	{
		item = new LastContactItem();
	}

	// update item data
	item->setItemId(sMsg.to());
	item->setItemName(sMsg.toName());
	item->setItemType(itemType);
	item->setLastBody(sMsg.messageBodyText());
	item->setLastTime(sMsg.time());
	item->setAttachment(sMsg.attachsCount());
	item->setTimeStamp(sMsg.stamp());
	item->setSendUid(sMsg.messageSendUid());

	// if this item is not in this model, add to this model
	if (!m_records.contains(sKey))
	{
		m_records[sKey] = item;
		appendRow(item);
	}

	// replace the db
	writeData(*item);

	// notify to update
	m_pProxyModel->invalidate();
}

void LastContactModel::replaceMsg(const bean::MessageBody &msg, const QString &stamp)
{
	if (stamp.isEmpty() || !msg.isValid())
		return;

	LastContactItem::LastContactItemType itemType = LastContactItem::LastContactTypeContact;
	switch (msg.messageType())
	{
	case bean::Message_Chat:
		itemType = LastContactItem::LastContactTypeContact;
		break;
	case bean::Message_GroupChat:
		itemType = LastContactItem::LastContactTypeGroupMuc;
		break;
	case bean::Message_DiscussChat:
		itemType = LastContactItem::LastContactTypeDiscuss;
		break;
	default:
		qDebug("%s messagetype invalid.", Q_FUNC_INFO);
		return;
	}

	QString sKey = makeKey(itemType, msg.to());

	// get the last contact item
	LastContactItem *item = 0;
	if (!m_records.contains(sKey))
		return;

	item = m_records[sKey];
	if (msg.time() != item->lastTime())
		return;

	QString origStamp = item->timeStamp();
	if (!origStamp.isEmpty() && stamp != origStamp)
		return;

	// update item data
	item->setLastBody(msg.messageBodyText());
	item->setAttachment(msg.attachsCount());

	// replace the db
	writeData(*item);

	// notify to update
	m_pProxyModel->invalidate();
}

void LastContactModel::appendMultiSendMsg(const bean::MessageBody &sMsg, const QString &id, const QStringList &members)
{
	if (id.isEmpty())
		return;

	QString sKey = makeKey(LastContactItem::LastContactTypeMultiSend, id);

	// get the last contact item
	LastContactItem *item = 0;
	if (m_records.contains(sKey))
	{
		item = m_records[sKey];
		if (sMsg.time() < item->lastTime())
		{
			// !!! do not accept the message which is earlier than the current one
			return;
		}
	}
	else
	{
		item = new LastContactItem();
	}

	item->setItemId(sMsg.to());
	item->setItemName(sMsg.toName());
	item->setItemType(stringToType(TYPE_MULTISEND));
	item->setLastBody(sMsg.messageBodyText());
	item->setLastTime(sMsg.time());
	item->setAttachment(sMsg.attachsCount());
	item->setMultiSendMembers(members);
	item->setTimeStamp(sMsg.stamp());
	item->setSendUid(sMsg.messageSendUid());

	// if this item is not in this model, add to this model
	if (!m_records.contains(sKey))
	{
		m_records[sKey] = item;
		appendRow(item);
	}

	// replace the db
	writeData(*item);

	// notify to update
	m_pProxyModel->invalidate();
}

QString LastContactModel::multiSendMsgId(const QStringList &members, bool &newCreated) const
{
	newCreated = false;
	foreach (LastContactItem *record, m_records.values())
	{
		if (record->itemType() == LastContactItem::LastContactTypeMultiSend)
		{
			QStringList recordMembers = record->multiSendMemebers();
			if (MsgMultiSendDlg::isSameMember(members, recordMembers))
			{
				return record->itemId();
			}
		}
	}

	newCreated = true;
	QString sUuid = QUuid::createUuid().toString();
	sUuid = sUuid.mid(1, sUuid.length()-2);
	return sUuid;
}

QSortFilterProxyModel *LastContactModel::proxyModel() const
{
	return m_pProxyModel;
}

LastContactItem *LastContactModel::nodeFromProxyIndex(const QModelIndex &proxyIndex)
{
	if (m_pProxyModel)
	{
		QModelIndex sourceIndex = m_pProxyModel->mapToSource(proxyIndex);
		if (sourceIndex.isValid())
		{
			return static_cast<LastContactItem *>(itemFromIndex(sourceIndex));
		}
		return 0;
	}
	else
	{
		return static_cast<LastContactItem *>(itemFromIndex(proxyIndex));
	}
}

LastContactItem *LastContactModel::nodeFromRow(int row)
{
	if (m_pProxyModel)
	{
		QModelIndex proxyIndex = m_pProxyModel->index(row, 0);
		QModelIndex sourceIndex = m_pProxyModel->mapToSource(proxyIndex);
		if (sourceIndex.isValid())
		{
			return static_cast<LastContactItem *>(itemFromIndex(sourceIndex));
		}
		return 0;
	}
	else
	{
		return static_cast<LastContactItem *>(item(row));
	}
}

bool LastContactModel::containsContact(const QString &id)
{
	QString sKey = makeKey(LastContactItem::LastContactTypeContact, id);
	return m_records.contains(sKey);
}

bool LastContactModel::containsMucGroup(const QString &id)
{
	QString sKey = makeKey(LastContactItem::LastContactTypeGroupMuc, id);
	return m_records.contains(sKey);
}

bool LastContactModel::containsDiscuss(const QString &id)
{
	QString sKey = makeKey(LastContactItem::LastContactTypeDiscuss, id);
	return m_records.contains(sKey);
}

LastContactItem *LastContactModel::contact(const QString &id) const
{
	LastContactItem *item = 0;
	QString sKey = makeKey(LastContactItem::LastContactTypeContact, id);
	if (m_records.contains(sKey))
		item = m_records[sKey];
	return item;
}

LastContactItem *LastContactModel::mucGroup(const QString &id) const
{
	LastContactItem *item = 0;
	QString sKey = makeKey(LastContactItem::LastContactTypeGroupMuc, id);
	if (m_records.contains(sKey))
		item = m_records[sKey];
	return item;
}

LastContactItem *LastContactModel::discuss(const QString &id) const
{
	LastContactItem *item = 0;
	QString sKey = makeKey(LastContactItem::LastContactTypeDiscuss, id);
	if (m_records.contains(sKey))
		item = m_records[sKey];
	return item;
}

QStringList LastContactModel::allContactIds() const
{
	QStringList contactIds;
	foreach (LastContactItem *item, m_records.values())
	{
		if (item->itemType() == LastContactItem::LastContactTypeContact)
		{
			contactIds << item->itemId();
		}
	}
	return contactIds;
}

QStringList LastContactModel::allMucGroupIds() const
{
	QStringList mucGroupIds;
	foreach (LastContactItem *item, m_records.values())
	{
		if (item->itemType() == LastContactItem::LastContactTypeGroupMuc)
		{
			mucGroupIds << item->itemId();
		}
	}
	return mucGroupIds;
}

QStringList LastContactModel::allDiscussIds() const
{
	QStringList discussIds;
	foreach (LastContactItem *item, m_records.values())
	{
		if (item->itemType() == LastContactItem::LastContactTypeDiscuss)
		{
			discussIds << item->itemId();
		}
	}
	return discussIds;
}

bool LastContactModel::hasUnreadMsg() const
{
	foreach (LastContactItem *lastContactItem, m_records.values())
	{
		if (lastContactItem->unreadMsgCount() > 0)
			return true;
	}
	return false;
}

bool LastContactModel::changeItemName(LastContactItem::LastContactItemType itemType, const QString &id, const QString &newName)
{
	QString sKey = makeKey(itemType, id);

	if (!m_records.contains(sKey))
		return false;

	LastContactItem *item = m_records[sKey];
	item->setItemName(newName);

	writeData(*item);

	return true;
}

void LastContactModel::checkDiscussNameChanged()
{
	QStringList discussIds = allDiscussIds();
	QString sKey;
	LastContactItem *item = 0;
	ModelManager *modelManager = qPmApp->getModelManager();
	foreach (QString discussId, discussIds)
	{
		sKey = makeKey(LastContactItem::LastContactTypeDiscuss, discussId);
		item = m_records[sKey];
		QString oldName = item->itemName();
		QString newName = modelManager->discussName(discussId);
		if (oldName != newName)
		{
			item->setItemName(newName);
			writeData(*item);
		}
	}
}

bool LastContactModel::readData()
{
	bool ret = false;
	do 
	{
		if (!m_pContactDB->isOpen())
		{
			if (!m_pContactDB->open())
			{
				break;
			}
		}

		QStringList fileds = m_pContactDB->fields(DB::LastContactDB::DB_LASTCONTACT_TABLENAME);

		QString sql = QString("select * from [%1] order by [lasttime] desc limit 100").arg(DB::LastContactDB::DB_LASTCONTACT_TABLENAME);
		QList<QVariantList> data = m_pContactDB->query(sql);

		ModelManager *modelManager = qPmApp->getModelManager();

		int nRow = 0;
		foreach (QVariantList vl, data)
		{
			if (vl.isEmpty() || vl.length() != fileds.length())
				continue;

			LastContactItem::LastContactItemType lastContactType = stringToType(vl[3].toString());
			QString id = vl[1].toString();

			if (id.isEmpty())
				continue;

			if (lastContactType == LastContactItem::LastContactTypeGroupMuc)
			{
				if (!modelManager->hasGroupItem(id))
					continue;
			}
			else if (lastContactType == LastContactItem::LastContactTypeDiscuss)
			{
				if (!modelManager->hasDiscussItem(id))
					continue;
			}
			/*
			else if (lastContactType == LastContactItem::LastContactTypeContact)
			{
				if (!modelManager->hasUserItem(id))
				continue;
			}
			*/

			// rebuild the last contact item
			LastContactItem *item = new LastContactItem(lastContactType, id, vl[2].toString());
			item->setLastBody(vl[4].toString());
			item->setLastTime(vl[5].toString());
			item->setAttachment(vl[6].toInt());
			QStringList members = vl[7].toString().split(",");
			item->setMultiSendMembers(members);
			item->setTimeStamp(vl[8].toString());
			item->setSendUid(vl[9].toString());
			
			// make key and add to records
			QString sKey = makeKey(item->itemType(), item->itemId());
			m_records[sKey] = item;

			// add this item to model
			appendRow(item);

			++nRow;
		}

		ret = true;

		if (nRow <= 0)
		{
			break;
		}

	} while (0);

	return ret;
}

void LastContactModel::writeData(const LastContactItem &item)
{
	cacheData(item);
}

QString LastContactModel::typeToString(LastContactItem::LastContactItemType itemType)
{
	QString typeStr(TYPE_CHAT);
	if (itemType == LastContactItem::LastContactTypeGroupMuc)
		typeStr = TYPE_GROUPCHAT;
	else if (itemType == LastContactItem::LastContactTypeDiscuss)
		typeStr = TYPE_DISCUSS;
	else if (itemType == LastContactItem::LastContactTypeMultiSend)
		typeStr = TYPE_MULTISEND;
	return typeStr;
}

LastContactItem::LastContactItemType LastContactModel::stringToType(const QString &str)
{
	LastContactItem::LastContactItemType type = LastContactItem::LastContactTypeContact;
	if (str == QString(TYPE_GROUPCHAT))
		type = LastContactItem::LastContactTypeGroupMuc;
	else if (str == QString(TYPE_DISCUSS))
		type = LastContactItem::LastContactTypeDiscuss;
	else if (str == QString(TYPE_MULTISEND))
		type = LastContactItem::LastContactTypeMultiSend;
	return type;
}

void LastContactModel::onUnreadItemCountChanged()
{
	UnreadMsgModel *unreadMsgModel = qPmApp->getUnreadMsgModel();
	if (unreadMsgModel)
	{
		// init to zero
		foreach (LastContactItem *lastContactItem, m_records.values())
		{
			lastContactItem->setUnreadMsgCount(0);
		}

		// check which one has un-read messages and set count
		QList<UnreadMsgItem *> unreadMsgs = unreadMsgModel->allUnreadMsgs();
		foreach (UnreadMsgItem *unreadMsg, unreadMsgs)
		{
			QString id = unreadMsg->id();
			bean::MessageType msgType = unreadMsg->msgType();
			int unreadMsgCount = unreadMsg->msgCount();

			OfflineMsgManager::FromType fType = OfflineMsgManager::User;

			QString sKey = "";
			if (msgType == bean::Message_Chat)
			{
				fType = OfflineMsgManager::User;
				sKey = makeKey(LastContactItem::LastContactTypeContact, id);
			}
			else if (msgType == bean::Message_GroupChat)
			{
				fType = OfflineMsgManager::Group;
				sKey = makeKey(LastContactItem::LastContactTypeGroupMuc, id);
			}
			else
			{
				fType = OfflineMsgManager::Discuss;
				sKey = makeKey(LastContactItem::LastContactTypeDiscuss, id);
			}

			int offlineMsgCount = qPmApp->getOfflineMsgManager()->offlineMsgCount(fType, id);
			unreadMsgCount += offlineMsgCount;

			if (m_records.contains(sKey))
			{
				LastContactItem *lastContactItem = m_records[sKey];
				lastContactItem->setUnreadMsgCount(unreadMsgCount);
			}
		}
	}
}

void LastContactModel::onRemoveChat(const QString &id)
{
	removeRecord(id, LastContactItem::LastContactTypeContact);
}

void LastContactModel::onRemoveGroupChat(const QString &id)
{
	removeRecord(id, LastContactItem::LastContactTypeGroupMuc);
}

void LastContactModel::onRemoveDiscuss(const QString &id)
{
	removeRecord(id, LastContactItem::LastContactTypeDiscuss);
}

void LastContactModel::onRemoveMultiSend(const QString &id)
{
	removeRecord(id, LastContactItem::LastContactTypeMultiSend);
}

void LastContactModel::onRemoveAllRecords()
{
	// clear write cache
	m_writeCache.clear();

	// clear db
	if (!m_records.isEmpty())
	{
		bool hasUnreadMsg = this->hasUnreadMsg();

		// clear all last contact items from model
		clear();
		m_records.clear();

		// clear from database
		if (!m_pContactDB->isOpen())
		{
			if (!m_pContactDB->open())
			{
				return;
			}
		}
		m_pContactDB->deleteRows(DB::LastContactDB::DB_LASTCONTACT_TABLENAME, QString(), QStringList());

		if (hasUnreadMsg)
		{
			emit unreadMsgRecordRemoved();
		}
	}
}

void LastContactModel::onMultiSendMemberChanged(const QString &id, const QString &name, const QStringList &members)
{
	if (id.isEmpty())
		return;

	QString sKey = makeKey(LastContactItem::LastContactTypeMultiSend, id);
	if (!m_records.contains(sKey))
		return;

	// get the last contact item
	LastContactItem *item = m_records[sKey];
	item->setItemName(name);
	item->setMultiSendMembers(members);

	// replace the db
	writeData(*item);

	// notify to update
	m_pProxyModel->invalidate();
}

void LastContactModel::onSubscriptionLastMsgChanged()
{
	QString sKey = makeKey(LastContactItem::LastContactTypeContact, QString(SUBSCRIPTION_ROSTER_ID));
	if (!m_records.contains(sKey))
		return;

	LastContactItem *item = m_records[sKey];
	SubscriptionLastMsgModel::LastMsgItem latestMsgItem = qPmApp->getModelManager()->subscriptionLastMsgModel()->latestMsg();
	if (!latestMsgItem.subId.isEmpty())
	{
		QString lastBody = latestMsgItem.lastBody;
		if (!latestMsgItem.send)
			lastBody = QString("%1: %2").arg(latestMsgItem.subName).arg(latestMsgItem.lastBody);
		item->setLastBody(lastBody);
		item->setLastTime(latestMsgItem.lastTime);
	}
	writeData(*item);
}

void LastContactModel::removeRecord(const QString &id, LastContactItem::LastContactItemType itemType)
{
	QString sKey = makeKey(itemType, id);

	// remove from write cache
	if (m_writeCache.contains(sKey))
	{
		delete m_writeCache[sKey];
		m_writeCache.remove(sKey);
	}

	// remove from db
	if (m_records.contains(sKey))
	{
		LastContactItem *item = m_records[sKey];
		bool hasUnreadMsg = (item->unreadMsgCount() > 0);

		// remove from model first
		this->takeRow(item->row());
		m_records.remove(sKey);
		delete item;
		item = 0;

		// remove from database
		if (!m_pContactDB->isOpen())
		{
			if (!m_pContactDB->open())
			{
				return;
			}
		}

		QStringList whereArg;
		whereArg << id << typeToString(itemType);
		m_pContactDB->deleteRows(DB::LastContactDB::DB_LASTCONTACT_TABLENAME, "uid=? AND type=?", whereArg);

		if (hasUnreadMsg)
		{
			emit unreadMsgRecordRemoved();
		}
	}
}

QString LastContactModel::makeKey(LastContactItem::LastContactItemType itemType, const QString &id)
{
	QString sKey;
	if (itemType == LastContactItem::LastContactTypeContact)
		sKey = QString(MAP_DIALOG_KEY_FORMAT).arg(TYPE_CHAT).arg(MAP_KEY_SEPARATION).arg(id);
	else if (itemType == LastContactItem::LastContactTypeGroupMuc)
		sKey = QString(MAP_DIALOG_KEY_FORMAT).arg(TYPE_GROUPCHAT).arg(MAP_KEY_SEPARATION).arg(id);
	else if (itemType == LastContactItem::LastContactTypeDiscuss)
		sKey = QString(MAP_DIALOG_KEY_FORMAT).arg(TYPE_DISCUSS).arg(MAP_KEY_SEPARATION).arg(id);
	else if (itemType == LastContactItem::LastContactTypeMultiSend)
		sKey = QString(MAP_DIALOG_KEY_FORMAT).arg(TYPE_MULTISEND).arg(MAP_KEY_SEPARATION).arg(id);
	return sKey;
}

void LastContactModel::cacheData(const LastContactItem &item)
{
	QString sKey = makeKey(item.itemType(), item.itemId());
	if (m_writeCache.contains(sKey))
	{
		delete m_writeCache[sKey];
		m_writeCache.remove(sKey);
	}
	LastContactItem *writeItem = new LastContactItem(item);
	m_writeCache.insert(sKey, writeItem);

	if (!m_writeTimer.isActive())
		m_writeTimer.start();
}

bool LastContactModel::commit(const LastContactItem &item)
{
	bool ret = false;
	do 
	{
		if (!m_pContactDB->isOpen())
		{
			if (!m_pContactDB->open())
			{
				break;
			}
		}

		QVariantMap vmap;
		vmap.insert("uid", item.itemId());
		vmap.insert("uname", item.itemName());
		vmap.insert("type", typeToString(item.itemType()));
		vmap.insert("lastbody", item.lastBody());
		vmap.insert("lasttime", item.lastTime());
		vmap.insert("hasAttachment", item.attachment());
		vmap.insert("members", item.multiSendMemebers().join(","));
		vmap.insert("timestamp", item.timeStamp());
		vmap.insert("senduid", item.sendUid());

		m_pContactDB->replace(DB::LastContactDB::DB_LASTCONTACT_TABLENAME, vmap);

		ret = true;
	} while (0);

	return ret;
}

void LastContactModel::commitAll()
{
	if (m_writeCache.isEmpty())
		return;

	foreach (QString sKey, m_writeCache.keys())
	{
		LastContactItem *writeItem = m_writeCache[sKey];
		commit(*writeItem);
		delete writeItem;
	}
	m_writeCache.clear();
}