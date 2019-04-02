#ifndef HTTPIMAGETOOLBUTTON_H
#define HTTPIMAGETOOLBUTTON_H

#include <QToolButton>
#include <QPointer>
#include <QVariantMap>
#include "widgetkit_global.h"

class QNetworkAccessManager;
class QNetworkReply;

class WIDGETKIT_EXPORT HttpImageToolButton : public QToolButton
{
	Q_OBJECT

public:
	HttpImageToolButton(QWidget *parent);
	~HttpImageToolButton();

	void setNetworkAccessManager(QNetworkAccessManager *networkAccessManager);
	void setCacheDir(const QString &cacheDir);
	void setHttpUrl(const QString &urlString);
	void setData(const QString &name, const QVariant &val);
	QVariant data(const QString &name) const;

private slots:
	void httpFinished(QNetworkReply *reply);

private:
	QString cacheFileName(const QString &urlString);
	void getImage();

private:
	QPointer<QNetworkAccessManager> m_networkAccessManager;
	QString                         m_cacheDir;
	QString                         m_urlString;
	QNetworkReply                  *m_networkReply;
	QVariantMap                     m_data;
};

#endif // HTTPIMAGETOOLBUTTON_H
