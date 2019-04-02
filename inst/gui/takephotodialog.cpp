#include "takephotodialog.h"
#include "ui_takephotodialog.h"
#include "session/VideoCapture.h"

TakePhotoDialog::TakePhotoDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::TakePhotoDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, false);

	setMainLayout(ui->verticalLayoutMain);
	setResizeable(true);
	setMaximizeable(true);

	resize(250, 314);
	setFixedSize(250, 314);
	setResizeable(false);

	m_videoCapture = new session::VideoCapture(this);

	m_videoRefreshTimer.setSingleShot(false);
	m_videoRefreshTimer.setInterval(200);
	connect(&m_videoRefreshTimer, SIGNAL(timeout()), this, SLOT(onRefreshTimeout()));
	m_videoRefreshTimer.start();

	m_numberTimer.setSingleShot(false);
	m_numberTimer.setInterval(1000);
	connect(&m_numberTimer, SIGNAL(timeout()), this, SLOT(onNumberTimeout()));

	ui->pushButtonOK->setVisible(true);
	ui->pushButtonRephoto->setVisible(false);
	ui->labelNum->setVisible(false);

	setSkin();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
}

TakePhotoDialog::~TakePhotoDialog()
{
	delete ui;
}

bool TakePhotoDialog::startCamera()
{
	m_videoCapture->startCapture();
	if (m_videoCapture->errorCode() != session::VideoCapture::Error_NoError)
		return false;
	return true;
}

void TakePhotoDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_8.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 32;
	bgSizes.bottomBarHeight = 31;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->labelTitle->setStyleSheet("QLabel {color: white;}");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->pushButtonOK->setStyleSheet(qss);
		ui->pushButtonRephoto->setStyleSheet(qss);
		qssFile.close();
	}
}

void TakePhotoDialog::onRefreshTimeout()
{
	QImage frame;
	m_videoCapture->getframe(frame);
	if (frame.isNull())
		return;

	QPixmap pixmap = QPixmap::fromImage(frame);
	pixmap = pixmap.scaled(ui->videoWidget->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui->videoWidget->setPixmap(pixmap);
}

void TakePhotoDialog::onNumberTimeout()
{
	++m_numberIndex;
	if (m_numberIndex >= 2)
	{
		m_numberTimer.stop();

		// show '1'
		ui->labelNum->setPixmap(QPixmap(":/images/photo_1.png"));

		takePhoto();
	}
	else
	{
		// show '2'
		ui->labelNum->setPixmap(QPixmap(":/images/photo_2.png"));
	}
}

void TakePhotoDialog::on_pushButtonOK_clicked()
{
	if (m_pixmap.isNull())
	{
		startPhoto();
	}
	else
	{
		accept();
	}
}

void TakePhotoDialog::on_pushButtonRephoto_clicked()
{
	m_pixmap = QPixmap();
	m_videoRefreshTimer.start();
	ui->pushButtonOK->setText(tr("Take photo"));
	ui->pushButtonOK->setVisible(true);
	ui->pushButtonRephoto->setVisible(false);
	ui->labelNum->setVisible(false);
}

void TakePhotoDialog::startPhoto()
{
	m_pixmap = QPixmap();
	m_numberIndex = 0;
	m_numberTimer.start();

	// show '3'
	ui->pushButtonOK->setVisible(false);
	ui->pushButtonRephoto->setVisible(false);
	ui->labelNum->setVisible(true);
	ui->labelNum->setPixmap(QPixmap(":/images/photo_3.png"));
}

void TakePhotoDialog::takePhoto()
{
	m_videoRefreshTimer.stop();
	m_pixmap = *ui->videoWidget->pixmap();

	ui->pushButtonOK->setVisible(true);
	ui->pushButtonRephoto->setVisible(true);
	ui->labelNum->setVisible(false);
	ui->pushButtonOK->setText(tr("Save"));
}