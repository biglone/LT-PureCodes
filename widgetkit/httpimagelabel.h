#ifndef HTTPIMAGELABEL_H
#define HTTPIMAGELABEL_H

#include <QLabel>
#include <QPointer>
#include "widgetkit_global.h"

class QNetworkAccessManager;
class QNetworkReply;

class WIDGETKIT_EXPORT HttpImageLabel : public QLabel
{
	Q_OBJECT

public:
	HttpImageLabel(QWidget *parent = 0);
	~HttpImageLabel();

	void setNetworkAccessManager(QNetworkAccessManager *networkAccessManager);
	void setCacheDir(const QString &cacheDir);
	void setHttpUrl(const QString &urlString);
	void setPixmapSize(const QSize &pixmapSize);

private slots:
	void httpFinished(QNetworkReply *reply);

private:
	QString cacheFileName(const QString &urlString);
	void getImage();

private:
	QPointer<QNetworkAccessManager> m_networkAccessManager;
	QString m_cacheDir;
	QString m_urlString;
	QSize   m_pixmapSize;
	QNetworkReply *m_networkReply;
};

#endif // HTTPIMAGELABEL_H
