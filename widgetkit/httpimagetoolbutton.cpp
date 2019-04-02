#include "httpimagetoolbutton.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QNetworkReply>
#include <QDebug>

HttpImageToolButton::HttpImageToolButton(QWidget *parent)
	: QToolButton(parent), m_networkReply(0)
{

}

HttpImageToolButton::~HttpImageToolButton()
{

}

void HttpImageToolButton::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
	m_networkAccessManager = networkAccessManager;
	connect(m_networkAccessManager.data(), SIGNAL(finished(QNetworkReply *)), 
		this, SLOT(httpFinished(QNetworkReply *)), Qt::UniqueConnection);
}

void HttpImageToolButton::setCacheDir(const QString &cacheDir)
{
	m_cacheDir = cacheDir;
}

void HttpImageToolButton::setHttpUrl(const QString &urlString)
{
	m_urlString = urlString;
	getImage();
}

void HttpImageToolButton::setData(const QString &name, const QVariant &val)
{
	m_data.insert(name, val);
}

QVariant HttpImageToolButton::data(const QString &name) const
{
	return m_data.value(name);
}

void HttpImageToolButton::httpFinished(QNetworkReply *reply)
{
	if (!reply)
		return;

	if (m_networkReply != reply)
		return;

	if (reply->error() != QNetworkReply::NoError)
	{
		qWarning() << Q_FUNC_INFO << "download image failed: " << reply->error() << reply->errorString();
		reply->deleteLater();
		m_networkReply = 0;
		return;
	}

	QByteArray data = reply->readAll();
	if (data.isEmpty())
	{
		qWarning() << Q_FUNC_INFO << "image is empty";
		reply->deleteLater();
		m_networkReply = 0;
		return;
	}

	QPixmap pixmap;
	if (!pixmap.loadFromData(data))
	{
		qWarning() << Q_FUNC_INFO << "load data to the pixmap failed";
		reply->deleteLater();
		m_networkReply = 0;
		return;
	}

	reply->deleteLater();
	m_networkReply = 0;

	QSize pixmapSize = iconSize();
	pixmap = pixmap.scaled(pixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	setIcon(pixmap);

	// save to file
	QString fileName = cacheFileName(m_urlString);
	if (fileName.isEmpty() || m_cacheDir.isEmpty())
		return;

	QDir cacheDir(m_cacheDir);
	QString filePath = QString("%1/%2").arg(m_cacheDir).arg(fileName);
	if (cacheDir.exists(fileName))
		QFile::remove(filePath);

	if (!pixmap.save(filePath))
	{
		qWarning() << Q_FUNC_INFO << "save pixmap failed: " << filePath;
	}
}

QString HttpImageToolButton::cacheFileName(const QString &urlString)
{
	QString fileName;
	if (urlString.isEmpty())
		return fileName;

	QUrl url = QUrl::fromUserInput(urlString);
	QFileInfo fileInfo(url.path());
	fileName = fileInfo.fileName();
	return fileName;
}

void HttpImageToolButton::getImage()
{
	if (m_urlString.isEmpty())
		return;

	QString fileName = cacheFileName(m_urlString);
	if (fileName.isEmpty() || m_cacheDir.isEmpty())
		return;

	// load from file
	QDir cacheDir(m_cacheDir);
	if (!cacheDir.exists())
		cacheDir.mkpath(m_cacheDir);
	QString filePath = QString("%1/%2").arg(m_cacheDir).arg(fileName);
	if (cacheDir.exists(fileName))
	{
		QPixmap pixmap;
		pixmap.load(filePath);
		QSize pixmapSize = iconSize();
		pixmap = pixmap.scaled(pixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		setIcon(pixmap);
		return;
	}

	// download from http
	if (!m_networkAccessManager.isNull())
	{
		QNetworkRequest request;
		request.setUrl(QUrl::fromUserInput(m_urlString));

		if (m_networkReply)
		{
			delete m_networkReply;
			m_networkReply = 0;
		}

		m_networkReply = m_networkAccessManager.data()->get(request);
	}
}

