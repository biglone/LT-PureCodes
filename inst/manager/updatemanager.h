#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QNetworkReply>
#include <QFile>

class QNetworkAccessManager;

class UpdateManager : public QObject
{
	Q_OBJECT

public:
	UpdateManager(QObject *parent = 0);
	~UpdateManager();

	void startUpdate(const QString &updateVersion, const QString &updateUrl);

signals:
	void downloadProgress(qint64 nBytesReceived, qint64 nBytesTotal);
	void downloadFinished(const QString &fileName);
	void downloadError(const QString &errMsg);

private slots:
	void onReadyRead();
	void onReplyError(QNetworkReply::NetworkError code);

private:
	QNetworkAccessManager *m_networkAccessManager;
	QNetworkReply         *m_networkReply;
	QString                m_updateVersion;
	QString                m_fileName;
	QFile                  m_file;
	qint64                 m_receivedBytes;
	qint64                 m_totalBytes;
};

#endif // UPDATEMANAGER_H
