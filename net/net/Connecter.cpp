#include <assert.h>
#include "TcpClient.h"
#include "Sender.h"
#include "Recver.h"
#include "Keeper.h"
#include "Connecter.h"
#include "ICallback.h"
#include <QTcpSocket>
#include <QSslSocket>
#include "cttk/Base.h"

#include "application/Logger.h"
using namespace application;

namespace net
{

Connecter::Connecter(TcpClient* pTcpClient)
: m_pTcpClient(pTcpClient)
{
	assert(m_pTcpClient != NULL);
}

Connecter::~Connecter()
{
	m_pTcpClient = 0;
}

void Connecter::connect()
{
	bool bConnect = false;
	QTcpSocket *pSocket = 0;

	// do connect
	do 
	{
		// create ssl client
		bool enableSsl = (m_pTcpClient->m_Param.encryptType >= TcpClient::TypeEncrypt_TLSV12);
		if (!enableSsl)
		{
			pSocket = new QTcpSocket();
			pSocket->connectToHost(m_pTcpClient->m_Param.address.ip.c_str(), m_pTcpClient->m_Param.address.port);
			if (!pSocket->waitForConnected(10*1000)) // max 10 seconds
			{
				LOG_ERROR(pSocket->errorString().toLocal8Bit().constData());
				break;
			}
		}
		else
		{
			QSslSocket *sslSocket = new QSslSocket();
			sslSocket->setProtocol(QSsl::TlsV1_2);
			sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
			if (!m_pTcpClient->m_Param.caFile.empty())
			{
				QString caFile = QString::fromUtf8(m_pTcpClient->m_Param.caFile.c_str());
				sslSocket->addCaCertificates(caFile);
			}
			if (!m_pTcpClient->m_Param.certFile.empty() && !m_pTcpClient->m_Param.keyFile.empty())
			{
				QString certFile = QString::fromUtf8(m_pTcpClient->m_Param.certFile.c_str());
				sslSocket->setLocalCertificate(certFile);

				QString keyFile = QString::fromUtf8(m_pTcpClient->m_Param.keyFile.c_str());
				sslSocket->setPrivateKey(keyFile);
				sslSocket->setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
			}

			// do connect
			pSocket = sslSocket;
			sslSocket->connectToHostEncrypted(m_pTcpClient->m_Param.address.ip.c_str(), m_pTcpClient->m_Param.address.port);
			if (!sslSocket->waitForEncrypted(10*1000)) // max 10 seconds
			{
				LOG_ERROR(sslSocket->errorString().toLocal8Bit().constData());
				break;
			}
		}

		bConnect = true;

	} while (0);

	if (!bConnect)
	{
		if (pSocket)
		{
			SAFE_DELETE(pSocket);
		}
	}
	else
	{
		m_pTcpClient->m_pTcpSocket = pSocket;
		bool connectOK = QObject::connect(pSocket, SIGNAL(readyRead()), m_pTcpClient->m_pRecver, SLOT(onReadyRead()));
		Q_ASSERT(connectOK);
		connectOK = QObject::connect(QThread::currentThread(), SIGNAL(finished()), pSocket, SLOT(deleteLater()));
		Q_ASSERT(connectOK);
		
		// start keeper
		Keeper* pKeeper = new Keeper(m_pTcpClient);
		m_pTcpClient->m_pKeeper = pKeeper;
		pKeeper->Start();
	}

	// notify connect
	ICallback* pICallback = m_pTcpClient->m_Param.pICallback;
	if (m_pTcpClient->m_bCallback && pICallback)
	{
		pICallback->onConnect(bConnect);
	}
}

}
