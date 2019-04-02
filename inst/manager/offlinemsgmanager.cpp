#include "offlinemsgmanager.h"
#include "pmclient/PmClient.h"
#include "protocol/ProtocolType.h"
#include "protocol/OfflineRequest.h"
#include "protocol/OfflineResponse.h"
#include "protocol/HistoryRequest.h"
#include "protocol/HistoryResponse.h"
#include "protocol/ReportTsRequest.h"
#include "protocol/ReportTsResponse.h"
#include <QTimer>
#include <QDebug>
#include "PmApp.h"
#include "MessageProcessor.h"
#include <string>
#include "protocol/MessageNotification.h"
#include "Account.h"
#include "settings/GlobalSettings.h"

static const int kOfflineRequestTimeout = 120;

OfflineMsgManager::OfflineMsgManager(QObject *parent)
	: QObject(parent), m_nHandleId(-1)
{
	m_ts.clear();
	m_tsNow.clear();
	m_offlineItems.clear();
}

OfflineMsgManager::~OfflineMsgManager()
{
}

void OfflineMsgManager::requestOfflineMsg(const QString &ts)
{
	m_ts = ts;
	protocol::OfflineRequest *request = new protocol::OfflineRequest(ts.toUtf8().constData());
	request->setTimeout(kOfflineRequestTimeout);
	PmClient::instance()->send(request);
}

void OfflineMsgManager::requestOfflineSyncMsg()
{
	protocol::OfflineRequest *request = new protocol::OfflineRequest(m_ts.toUtf8().constData(), m_ts2.toUtf8().constData());
	request->setTimeout(kOfflineRequestTimeout);
	PmClient::instance()->send(request);
}

bool OfflineMsgManager::requestHistoryMsg(FromType fType, const QString &fromId)
{
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		OfflineItem offlineItem = m_offlineItems[i];
		FromType offlineFromType = offlineItem.m_type;
		QString offlineFromId = offlineItem.m_from;
		if (offlineFromType == fType && offlineFromId == fromId)
		{
			// fix history message count
			int msgCount = PAGE_MSG_COUNT;
			if (offlineItem.m_offlineCount > 0)
			{
				if (msgCount > offlineItem.m_offlineCount)
					msgCount = offlineItem.m_offlineCount;
			}
			else if (offlineItem.m_count > 0)
			{
				if (msgCount > offlineItem.m_count)
					msgCount = offlineItem.m_count;
			}

			// get ts
			QString ts = offlineItem.m_ts;

			// send history request
			QString bareFromId = Account::idFromFullId(fromId);
			protocol::HistoryRequest *request = new protocol::HistoryRequest((protocol::OfflineResponse::ItemType)fType, 
				bareFromId.toUtf8().constData(), ts.toUtf8().constData(), (protocol::HistoryRequest::Direction)Backward, msgCount);
			request->setTimeout(kOfflineRequestTimeout);
			PmClient::instance()->send(request);

			return true;
		}
	}

	return false;
}

void OfflineMsgManager::clear()
{
	m_ts.clear();
	m_tsNow.clear();
	m_offlineItems.clear();
	emit offlineChanged();
}

QList<OfflineMsgManager::OfflineItem> OfflineMsgManager::offlineItems() const
{
	return m_offlineItems;
}

bool OfflineMsgManager::containOfflineItem(FromType fType, const QString &fromId) const
{
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		if (m_offlineItems[i].m_type == fType && m_offlineItems[i].m_from == fromId)
		{
			return true;
		}
	}
	return false;
}

int OfflineMsgManager::offlineMsgCount(FromType fType, const QString &fromId) const
{
	int count = 0;
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		if (m_offlineItems[i].m_type == fType && m_offlineItems[i].m_from == fromId)
		{
			count = m_offlineItems[i].m_offlineCount;
			break;
		}
	}

	if (count < 0)
		count = 0;
	return count;
}

void OfflineMsgManager::clearOfflineMsgCount(FromType fType, const QString &fromId)
{
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		if (m_offlineItems[i].m_type == fType && m_offlineItems[i].m_from == fromId)
		{
			m_offlineItems[i].m_offlineCount = 0;
			emit offlineChanged();
			return;
		}
	}
}

int OfflineMsgManager::syncMsgCount(FromType fType, const QString &fromId) const
{
	int count = 0;
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		if (m_offlineItems[i].m_type == fType && m_offlineItems[i].m_from == fromId)
		{
			count = m_offlineItems[i].m_count;
			break;
		}
	}

	if (count < 0)
		count = 0;
	return count;
}

bool OfflineMsgManager::clearOfflineItem(FromType fType, const QString &fromId)
{
	bool removed = false;
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		if (m_offlineItems[i].m_type == fType && m_offlineItems[i].m_from == fromId)
		{
			m_offlineItems.removeAt(i);
			removed = true;
			emit offlineChanged();
			break;
		}
	}
	return removed;
}

bean::MessageType OfflineMsgManager::offlineFromType2MessageType(FromType fromType)
{
	if (fromType == Group)
		return bean::Message_GroupChat;
	else if (fromType == Discuss)
		return bean::Message_DiscussChat;
	else
		return bean::Message_Chat;
}

OfflineMsgManager::FromType OfflineMsgManager::messageType2OfflineFromType(bean::MessageType msgType)
{
	if (msgType == bean::Message_GroupChat)
		return Group;
	else if (msgType == bean::Message_DiscussChat)
		return Discuss;
	else
		return User;
}

void OfflineMsgManager::reportMaxTs(const QString &maxTs)
{
	protocol::ReportTsRequest *request = new protocol::ReportTsRequest(maxTs.toUtf8().constData());
	PmClient::instance()->send(request);
}

bool OfflineMsgManager::initObject()
{
	m_nHandleId = PmClient::instance()->insertResponseHandler(this);
	if (m_nHandleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}
	
	qWarning() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void OfflineMsgManager::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* OfflineMsgManager::instance()
{
	return this;
}

int OfflineMsgManager::handledId() const
{
	return m_nHandleId;
}

QList<int> OfflineMsgManager::types() const
{
	QList<int> ret;
	ret << protocol::Request_Msg_Offline;
	ret << protocol::Request_Msg_History;
	ret << protocol::Request_Report_Ts;
	return ret;
}

bool OfflineMsgManager::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	if (m_nHandleId != handleId)
	{
		return false;
	}

	int type = req->getType();
	do 
	{
		// process
		switch (type)
		{
		case protocol::Request_Msg_Offline:
			processOfflineMsg(req, res);
			break;
		case protocol::Request_Msg_History:
			processHistoryMsg(req, res);
			break;
		case protocol::Request_Report_Ts:
			processReportTs(req, res);
			break;
		default:
			qWarning() << Q_FUNC_INFO << "error";
		}
	} while (0);

	return true;
}

void OfflineMsgManager::offlineFailed()
{
	m_offlineItems.clear();

	emit offlineRecvOK();
}

void OfflineMsgManager::offlineFinished()
{
	if (GlobalSettings::isOfflineSyncMsgEnabled())
	{
		if (!m_ts.isEmpty() && !m_ts2.isEmpty() && m_ts < m_ts2)
		{
			qlonglong nTs2 = m_ts2.toLongLong();
			nTs2 += 1;
			m_ts2 = QString("%1").arg(nTs2, 29, 10, QLatin1Char('0'));
			requestOfflineSyncMsg();
		}
		else
		{
			offlineSyncFinished();
		}
	}
	else
	{
		offlineSyncFinished();
	}
}

void OfflineMsgManager::offlineSyncFinished()
{
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		OfflineItem offlineItem = m_offlineItems[i];
		requestHistoryMsg(offlineItem.m_type, offlineItem.m_from);
	}

	emit offlineRecvOK();
}

void OfflineMsgManager::historyMsgReceived(int fType, const QString &bareFromId, int number, const bean::MessageBodyList &messages)
{
	QString fromId = bareFromId;
	if (fromId == Account::instance()->id())
		fromId = Account::instance()->phoneFullId();

	// update the off-line items
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		OfflineItem offlineItem = m_offlineItems[i];
		int offlineFType = offlineItem.m_type;
		QString offlineFromId = offlineItem.m_from;
		if (fType == offlineFType && fromId == offlineFromId)
		{
			bool offline = true;
			if (offlineItem.m_offlineCount <= 0)
				offline = false;
			offlineItem.m_count -= number; // !!! here use number other than messages.count(), because messages.count() may be less than number
			offlineItem.m_offlineCount -= number;
			if (offlineItem.m_offlineCount < 0)
				offlineItem.m_offlineCount = 0;
			if (messages.count() > 0)
			{
				offlineItem.m_ts = messages[0].stamp();
			}
			if (offlineItem.m_count > 0) // still has message, update the message count
			{
				m_offlineItems[i] = offlineItem;
			}
			else // no message, remove this item
			{
				m_offlineItems.removeAt(i);
			}

			// notify history message received
			emit historyMsgRecvOK(fType, fromId, messages, offline);

			return;
		}
	}
}

void OfflineMsgManager::historyMsgFailed(int fType, const QString &bareFromId)
{
	QString fromId = bareFromId;
	if (fromId == Account::instance()->id())
		fromId = Account::instance()->phoneFullId();

	// update the off-line items
	for (int i = 0; i < m_offlineItems.count(); i++)
	{
		OfflineItem offlineItem = m_offlineItems[i];
		int offlineFType = offlineItem.m_type;
		QString offlineFromId = offlineItem.m_from;
		if (fType == offlineFType && fromId == offlineFromId)
		{
			// message failed, remove this item
			m_offlineItems.removeAt(i);
			break;
		}
	}

	// notify history message received
	emit historyMsgRecvFailed(fType, fromId);
}

void OfflineMsgManager::processOfflineMsg(net::Request* req, protocol::Response* res)
{
	if (processResponseError(req)) // error
	{
		QMetaObject::invokeMethod(this, "offlineFailed", Qt::QueuedConnection);
		return;
	}

	protocol::OfflineResponse *pRes = static_cast<protocol::OfflineResponse *>(res);
	Q_ASSERT(pRes != NULL);

	std::string ts = pRes->getTs();
	std::string ts2 = pRes->getTs2();
	std::string tsNow = pRes->getTsNow();
	std::list<protocol::OfflineResponse::Item> items = pRes->getItems();

	if (ts2.empty())
	{
		m_ts2 = QString::fromUtf8(ts.c_str());
		m_ts2 = m_ts2.trimmed();
		m_tsNow = QString::fromUtf8(tsNow.c_str());
		m_tsNow = m_tsNow.trimmed();
		for (std::list<protocol::OfflineResponse::Item>::iterator it = items.begin(); it != items.end(); ++it)
		{
			protocol::OfflineResponse::Item item = *it;
			OfflineItem offlineItem;
			offlineItem.m_type = (FromType)item.m_type;
			offlineItem.m_from = QString::fromUtf8(item.m_from.c_str());
			offlineItem.m_offlineCount = item.m_count;
			offlineItem.m_count = item.m_count;
			offlineItem.m_ts = m_tsNow;
			if (offlineItem.m_from == Account::instance()->id())
				offlineItem.m_from = Account::instance()->phoneFullId();
			m_offlineItems.append(offlineItem);
		}

		QMetaObject::invokeMethod(this, "offlineFinished", Qt::QueuedConnection);
	}
	else
	{
		for (std::list<protocol::OfflineResponse::Item>::iterator it = items.begin(); it != items.end(); ++it)
		{
			protocol::OfflineResponse::Item item = *it;
			OfflineItem offlineItem;
			offlineItem.m_type = (FromType)item.m_type;
			offlineItem.m_from = QString::fromUtf8(item.m_from.c_str());
			offlineItem.m_offlineCount = 0;
			offlineItem.m_count = item.m_count;
			offlineItem.m_ts = m_ts2;
			if (offlineItem.m_from == Account::instance()->id())
			{
				offlineItem.m_from = Account::instance()->phoneFullId();
				offlineItem.m_offlineCount = item.m_count;
			}

			int i = 0;
			for (i = 0; i < m_offlineItems.count(); ++i)
			{
				if (m_offlineItems[i].m_type == offlineItem.m_type && m_offlineItems[i].m_from == offlineItem.m_from)
				{
					m_offlineItems[i].m_offlineCount += offlineItem.m_offlineCount;
					m_offlineItems[i].m_count += offlineItem.m_count;
					break;
				}
			}

			if (i >= m_offlineItems.count())
			{
				m_offlineItems.append(offlineItem);
			}
		}

		QMetaObject::invokeMethod(this, "offlineSyncFinished", Qt::QueuedConnection);
	}
}

void OfflineMsgManager::processHistoryMsg(net::Request* req, protocol::Response* res)
{
	if (processResponseError(req)) // error
	{
		protocol::HistoryRequest *historyRequest = static_cast<protocol::HistoryRequest *>(req);
		if (historyRequest)
		{
			int fType = (int)historyRequest->fromType();
			QString fromId = QString::fromUtf8(historyRequest->fromId().c_str());
			QMetaObject::invokeMethod(this, "historyMsgFailed", Qt::QueuedConnection, Q_ARG(int, fType), Q_ARG(QString, fromId));
		}
		return;
	}

	protocol::HistoryResponse *pRes = static_cast<protocol::HistoryResponse *>(res);
	Q_ASSERT(pRes != NULL);

	int fType = (int)pRes->fType();
	QString fromId = QString::fromUtf8(pRes->from().c_str());
	int number = pRes->number();

	bean::MessageBodyList messageBodys;
	std::list<protocol::MessageNotification::Message> messages = pRes->getMessages();
	for (std::list<protocol::MessageNotification::Message>::iterator it = messages.begin(); it != messages.end(); ++it)
	{
		protocol::MessageNotification::Message message = *it;
		bean::MessageBody messageBody = qPmApp->getMessageProcessor()->message2MsgBody(&message);
		messageBodys.append(messageBody);
	}

	QMetaObject::invokeMethod(this, "historyMsgReceived", Qt::QueuedConnection, 
		Q_ARG(int, fType), Q_ARG(QString, fromId), Q_ARG(int, number), Q_ARG(bean::MessageBodyList, messageBodys));
}

void OfflineMsgManager::processReportTs(net::Request* req, protocol::Response* /*res*/)
{
	if (processResponseError(req)) // error
	{
		QMetaObject::invokeMethod(this, "reportTsFailed", Qt::QueuedConnection);
		return;
	}

	QMetaObject::invokeMethod(this, "reportTsOk", Qt::QueuedConnection);
}

bool OfflineMsgManager::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		QString sReqTitle;
		if (req->getType() == protocol::Request_Msg_Offline)
			sReqTitle = "Offline request";
		else if (req->getType() == protocol::Request_Msg_History)
			sReqTitle = "History request";
		else
			sReqTitle = "ReportTs request";
		QString sError = QString::fromUtf8(req->getMessage().c_str());
		QString errMsg = QString("%1%2(%3)").arg(sReqTitle).arg(" error: ").arg(sError);
		qWarning() << errMsg;
	}

	return bError;
}