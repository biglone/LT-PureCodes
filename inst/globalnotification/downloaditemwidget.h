#ifndef DOWNLOADITEMWIDGET_H
#define DOWNLOADITEMWIDGET_H

#include <QWidget>
#include <QTime>
#include "ui_downloaditemwidget.h"
#include <QFile>
#include <QNetworkReply>
#include <QTimer>

class QNetworkReply;
class QNetworkAccessManager;

class DownloadItemWidget : public QWidget
{
	Q_OBJECT

public:
	DownloadItemWidget(const QString saveFileName, QNetworkReply *reply, QNetworkAccessManager * nam, bool autoOpen = false, QWidget *parent = 0);
	~DownloadItemWidget();

	enum Status{
		Status_Downloading,
		Status_Stoped,
		Status_Failed,
		Status_Successfully
	};
	Status status() const {return m_status;}

	bool isChecked();
	void setChecked(bool checked);

	void flash();

protected:
	void mouseReleaseEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *e);
	void init();

public slots:
	void open();
	void openFolder();
	void setAutoOpen(bool bAutoOpen){m_bAutoOpen = bAutoOpen;}
	void stop();
	void tryAgain();

	void downloadReadyRead();
	void error(QNetworkReply::NetworkError code);
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void metaDataChanged();
	void finished();

private slots:
	void flashTimeout();

private:
	void updateInfoLabel();
	QString dataString(int size) const;
	void setStatus(Status s);
	void initUI();

private:
	Ui::DownloadItemWidget ui;

	QFile          m_output;
	QNetworkReply *m_reply;

	qint64 m_bytesReceived;
	qint64 m_bytesTotal;
	qint64 m_lastBytesReceived;
	QTime  m_downloadTime;
	QUrl   m_url;
	
	Status m_status;
	bool   m_bAutoOpen;
	QNetworkAccessManager *m_nam;

	int    m_flashIndex;
	QTimer m_flashTimer;
};

#endif // DOWNLOADITEMWIDGET_H
