#include "tipmanager.h"
#include "pmclient/PmClient.h"
#include "protocol/ProtocolType.h"
#include "protocol/TipNotification.h"
#include <QDebug>

static const char *kInputTipType = "input";
static const char *kSpeakTipType = "speak";

TipManager::TipManager(QObject *parent /*= 0*/)
	: QObject(parent), m_nHandleId(-1)
{

}

TipManager::~TipManager()
{

}

void TipManager::sendInputTip(const QString &from, const QString &to, const QString &action /*= QString()*/)
{
	if (from.isEmpty() || to.isEmpty())
		return;

	sendTip(kInputTipType, from, to, action);
}

void TipManager::sendSpeakTip(const QString &from, const QString &to, const QString &action /*= QString()*/)
{
	if (from.isEmpty() || to.isEmpty())
		return;

	sendTip(kSpeakTipType, from, to, action);
}

bool TipManager::initObject()
{
	m_nHandleId = PmClient::instance()->insertNotificationHandler(this);
	if (m_nHandleId < 0)
	{
		qWarning() << Q_FUNC_INFO << "insert handle error.";
		return false;
	}

	qDebug() << Q_FUNC_INFO << " handle: " << m_nHandleId;
	return true;
}

void TipManager::removeObject()
{
	PmClient::instance()->removeNotificationHandler(m_nHandleId);
	m_nHandleId = -1;
}

QObject* TipManager::instance()
{
	return this;
}

QList<int> TipManager::types() const
{
	return QList<int>() << protocol::TIP;
}

int TipManager::handledId() const
{
	return m_nHandleId;
}

bool TipManager::onNotication(int handleId, protocol::SpecificNotification* sn)
{
	if (m_nHandleId != handleId)
		return false;

	protocol::TipNotification::In *pIn = static_cast<protocol::TipNotification::In *>(sn);

	if (pIn)
	{
		QString from = QString::fromUtf8(pIn->from().c_str());
		QString to = QString::fromUtf8(pIn->to().c_str());
		QString type = QString::fromUtf8(pIn->type().c_str());
		QString action = QString::fromUtf8(pIn->action().c_str());
		QMetaObject::invokeMethod(this, "processTip", 
			Q_ARG(QString, from),
			Q_ARG(QString, to),
			Q_ARG(QString, type),
			Q_ARG(QString, action));
	}

	return true;
}

void TipManager::processTip(const QString &from, const QString &to, const QString &type, const QString &action)
{
	if (type == kInputTipType)
	{
		emit inputTipRecved(from, to, action);
	}
	else if (type == kSpeakTipType)
	{
		emit speakTipRecved(from, to, action);
	}
	else
	{
		qWarning() << Q_FUNC_INFO << "other type: " << type << from << to << action;
	}
}


void TipManager::sendTip(const QString &type, const QString &from, const QString &to, const QString &action)
{
	protocol::TipNotification::Out *pOut = new protocol::TipNotification::Out(
		from.toUtf8().constData(), to.toUtf8().constData(), type.toUtf8().constData(), action.toUtf8().constData());
	PmClient::instance()->send(pOut);
}