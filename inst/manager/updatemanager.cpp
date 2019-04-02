#include "updatemanager.h"
#include <QDir>
#include "Constants.h"
#include <QUrl>
#include <QDebug>

UpdateManager::UpdateManager(QObject *parent)
	: QObject(parent), m_networkReply(0), m_receivedBytes(0), m_totalBytes(0)
{
	m_networkAccessManager = new QNetworkAccessManager(this);
}

UpdateManager::~UpdateManager()
{
	if (m_networkReply)
	{
		delete m_networkReply;
		m_networkReply = 0;
	}
}

void UpdateManager::startUpdate(const QString &updateVersion, const QString &updateUrl)
{
	if (updateUrl.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "update url is empty";
		return;
	}

	m_updateVersion = updateVersion;
	m_fileName = QString("%1/%2_%3.exe").arg(QDir::tempPath()).arg(APP_NAME).arg(m_updateVersion);
	if (m_file.isOpen())
	{
		m_file.close();
	}
	m_file.setFileName(m_fileName);
	if (m_file.exists())
	{
		m_file.remove();
	}
	if (!m_file.open(QIODevice::WriteOnly))
	{
		emit downloadError(tr("Create update file failed"));
		return;
	}

	m_receivedBytes = 0;
	m_totalBytes = 0;

	QNetworkRequest request;
	request.setUrl(QUrl(updateUrl));
	qDebug() << Q_FUNC_INFO << updateUrl;

	m_networkReply = m_networkAccessManager->get(request);
	connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onReplyError(QNetworkReply::NetworkError)));
	connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void UpdateManager::onReadyRead()
{
	if (m_receivedBytes == 0)
	{
		if (!m_networkReply->hasRawHeader("Content-length"))
		{
			m_file.close();

			m_networkReply->deleteLater();
			m_networkReply = 0;

			qWarning() << Q_FUNC_INFO << "http response error: no file length";
			emit downloadError(tr("Response error: no file length"));
			return;
		}

		QByteArray lengthHeader = m_networkReply->rawHeader("Content-length");
		m_totalBytes = QString(lengthHeader).toLongLong();
	}

	QByteArray content = m_networkReply->readAll();

	if (m_file.isOpen())
	{
		m_file.write(content);
	}

	m_receivedBytes += content.length();

	emit downloadProgress(m_receivedBytes, m_totalBytes);

	if (m_receivedBytes >= m_totalBytes)
	{
		m_file.close();

		m_networkReply->deleteLater();
		m_networkReply = 0;

		emit downloadFinished(m_fileName);
	}
}

void UpdateManager::onReplyError(QNetworkReply::NetworkError code)
{
	m_file.close();
	if (m_networkReply)
	{
		m_networkReply->deleteLater();
		m_networkReply = 0;
	}

	qWarning() << Q_FUNC_INFO << code;
	QString errorString = tr("Network error:%1").arg((int)code);
	emit downloadError(errorString);
}
