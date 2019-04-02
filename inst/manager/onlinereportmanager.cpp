#include "onlinereportmanager.h"
#include "pmclient/PmClient.h"
#include "protocol/ProtocolType.h"
#include "protocol/ReportOnlineRequest.h"
#include "protocol/ReportOnlineResponse.h"
#include <QDebug>

OnlineReportManager::OnlineReportManager(QObject *parent /*= 0*/)
	: QObject(parent), m_nHandleId(-1)
{
	m_timer.setInterval(30*60*1000); // 30min
	m_timer.setSingleShot(false);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(reportOnline()));
}

OnlineReportManager::~OnlineReportManager()
{

}

void OnlineReportManager::start(const QString &id)
{
	m_id = id;
	if (m_id.isEmpty())
		return;

	reportOnline();
	m_timer.start();
}

void OnlineReportManager::stop()
{
	m_timer.stop();
}

void OnlineReportManager::reportOnline()
{
	if (m_id.isEmpty())
		return;

	protocol::ReportOnlineRequest *request = new protocol::ReportOnlineRequest(m_id.toUtf8().constData(), "online");
	PmClient::instance()->send(request);
}

void OnlineReportManager::reportOffline()
{
	if (m_id.isEmpty())
		return;

	protocol::ReportOnlineRequest *request = new protocol::ReportOnlineRequest(m_id.toUtf8().constData(), "offline");
	PmClient::instance()->send(request);
}

bool OnlineReportManager::initObject()
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

void OnlineReportManager::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* OnlineReportManager::instance()
{
	return this;
}

int OnlineReportManager::handledId() const
{
	return m_nHandleId;
}

QList<int> OnlineReportManager::types() const
{
	QList<int> ret;
	ret << protocol::Request_Report_Online;
	return ret;
}

bool OnlineReportManager::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	if (m_nHandleId != handleId)
	{
		return false;
	}

	int type = req->getType();
	// process
	switch (type)
	{
	case protocol::Request_Report_Online:
		processReportOnline(req, res);
		break;
	default:
		qWarning() << Q_FUNC_INFO << "error";
	}

	return true;
}

void OnlineReportManager::processReportOnline(net::Request* req, protocol::Response* /*res*/)
{
	if (processResponseError(req)) // error
	{
		QMetaObject::invokeMethod(this, "reportOnlineFailed", Qt::QueuedConnection);
		return;
	}

	QMetaObject::invokeMethod(this, "reportOnlineOK", Qt::QueuedConnection);
}

bool OnlineReportManager::processResponseError(net::Request* req)
{
	bool bError = !req->getResult();

	if (bError)
	{
		QString sError = QString::fromUtf8(req->getMessage().c_str());
		QString errMsg = QString("Report online error: (%3)").arg(sError);
		qWarning() << errMsg;
	}

	return bError;
}