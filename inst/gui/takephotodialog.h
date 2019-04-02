#ifndef TAKEPHOTODIALOG_H
#define TAKEPHOTODIALOG_H

#include "framelessdialog.h"
#include <QTimer>
#include <QPixmap>

namespace Ui {class TakePhotoDialog;};
namespace session {class VideoCapture;};

class TakePhotoDialog : public FramelessDialog
{
	Q_OBJECT

public:
	TakePhotoDialog(QWidget *parent = 0);
	~TakePhotoDialog();

	bool startCamera();

	QPixmap photo() const {return m_pixmap;}

public slots:
	void setSkin();

private slots:
	void onRefreshTimeout();
	void onNumberTimeout();
	void on_pushButtonOK_clicked();
	void on_pushButtonRephoto_clicked();

private:
	void startPhoto();
	void takePhoto();

private:
	Ui::TakePhotoDialog *ui;

	session::VideoCapture *m_videoCapture;
	QTimer  m_videoRefreshTimer;
	QPixmap m_pixmap;
	QTimer  m_numberTimer;
	int     m_numberIndex;
};

#endif // TAKEPHOTODIALOG_H
