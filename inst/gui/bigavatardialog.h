#ifndef BIGAVATARDIALOG_H
#define BIGAVATARDIALOG_H

#include "framelessdialog.h"
#include <QByteArray>
#include <QPointer>
#include <QPixmap>
#include <QTimer>
namespace Ui {class BigAvatarDialog;};
class QNetworkAccessManager;
class QNetworkReply;

class BigAvatarDialog : public FramelessDialog
{
	Q_OBJECT

public:
	BigAvatarDialog(QWidget *parent = 0);
	~BigAvatarDialog();

	static BigAvatarDialog *getDialog();

public slots:
	void setSkin();

	void getBigAvatar(const QString &uid);

private slots:
	void refresh();
	void onReadyRead();
	void onReplyFinished();
	void onReplyTimeout();

private:
	void initUI();
	void setAvatar(const QPixmap &pixmap);

private:
	Ui::BigAvatarDialog *ui;

	static QPointer<BigAvatarDialog> s_dialog;

	QString                m_uid;
	QNetworkAccessManager *m_networkAccessManager;
	QNetworkReply         *m_networkReply;
	QByteArray             m_imageBuffer;
	int                    m_imageLength;
	QTimer                 m_networkTimer;
};

#endif // BIGAVATARDIALOG_H
