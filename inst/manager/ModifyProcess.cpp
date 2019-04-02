#include <QDebug>
#include <QBuffer>
#include <QImage>
#include "pmclient/PmClient.h"

#include "protocol/ProtocolType.h"
#include "protocol/ProtocolConst.h"
#include "protocol/ModifyMessage.h"

#include "bean/DetailItem.h"
#include "ModifyProcess.h"

ModifyProcess::ModifyProcess(QObject *parent)
	: QObject(parent)
	, m_handleId(-1)
{

}

ModifyProcess::~ModifyProcess()
{

}

bool ModifyProcess::sendModify(const QMap<int, QVariant>& vals)
{
	protocol::ModifyMessage::ModifyRequest* req = new protocol::ModifyMessage::ModifyRequest();
	foreach (int role, vals.keys())
	{
		QString sVal;
		std::string key = bean::DetailItem::detailDataRole2String((bean::DetailDataRole)role);
		if (key.empty())
			continue;

		if (bean::DETAIL_PHOTO == role)
		{
			// modify via http
			/*
			QImage img = qvariant_cast<QImage>(vals.value(role));

			QBuffer buf;
			if (!img.save(&buf, "jpg"))
				continue;

			QByteArray ba = buf.buffer().toBase64();
			req->addContent(key, ba.data());
			*/
		}
		else
		{
			QString sVal = vals.value(role).toString();
			req->addContent(key, sVal.toUtf8().constData());
		}
	}

	return PmClient::instance()->send(req);
}

bool ModifyProcess::initObject()
{
	m_handleId = PmClient::instance()->insertResponseHandler(this);
	if (m_handleId < 0)
	{
		qWarning() << Q_FUNC_INFO << " insert handle error.";
		return false;
	}

	qWarning() << Q_FUNC_INFO << " handle: " << m_handleId;
	return true;
}

void ModifyProcess::removeObject()
{
	PmClient::instance()->removeResponseHandler(m_handleId);
	m_handleId = -1;
}

QObject* ModifyProcess::instance()
{
	return this; 
}

int ModifyProcess::handledId() const
{
	return m_handleId; 
}

QList<int> ModifyProcess::types() const
{
	QList<int> ret;
	ret << protocol::Request_IM_Modify;

	return ret;
}

bool ModifyProcess::onRequestResult(int handleId, net::Request* req, protocol::Response* res)
{
	Q_UNUSED(res);
	if (handleId != m_handleId)
		return false;

	do 
	{
		if (!req->getResult())
		{
			// error
			QString sReqTitle = tr("Modify request");

			emit error(tr("%1 failed(%2)").arg(sReqTitle).arg(QString::fromUtf8(req->getMessage().c_str())));
			break;
		}

		// process
		emit finish();
	} while (0);

	return false;
}