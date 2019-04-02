#include "bigavatardialog.h"
#include "ui_bigavatardialog.h"
#include <QMovie>
#include "settings/GlobalSettings.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "PmApp.h"
#include "ModelManager.h"
#include "detailphotomanager.h"
#include <QDebug>
#include <QDesktopWidget>
#include "http/HttpPool.h"

static const int kTitleBarHeight = 33;
static const int kInitImageSize  = 330;
static const int kBorder         = 5;

QPointer<BigAvatarDialog> BigAvatarDialog::s_dialog;

BigAvatarDialog::BigAvatarDialog(QWidget *parent /*= 0*/)
	: FramelessDialog(parent), m_networkAccessManager(0), m_networkReply(0), m_imageLength(0)
{
	ui = new Ui::BigAvatarDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::FramelessWindowHint;
	setWindowFlags(flags);

	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	resize(kInitImageSize+2*kBorder, kTitleBarHeight+kInitImageSize+2*kBorder);
	setResizeable(false);
	setMaximizeable(false);

	initUI();

	m_networkAccessManager = new QNetworkAccessManager(this);
	m_networkTimer.setSingleShot(true);
	m_networkTimer.setInterval(30*1000);
	connect(&m_networkTimer, SIGNAL(timeout()), this, SLOT(onReplyTimeout()));

	setSkin();

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->labelRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
}

BigAvatarDialog::~BigAvatarDialog()
{
	if (m_networkReply)
	{
		delete m_networkReply;
		m_networkReply = 0;
	}

	delete ui;
}

BigAvatarDialog *BigAvatarDialog::getDialog()
{
	if (s_dialog.isNull())
	{
		s_dialog = new BigAvatarDialog();
	}
	return s_dialog.data();
}

void BigAvatarDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	ui->title->setStyleSheet("font-size: 12pt; color: white;");
	ui->labelLoadTip->setStyleSheet("color: rgb(128, 128, 128);");
	ui->labelErrorTip->setStyleSheet("color: rgb(128, 128, 128);");
	ui->labelNoImageTip->setStyleSheet("color: rgb(128, 128, 128);");
}

void BigAvatarDialog::getBigAvatar(const QString &uid)
{
	if (uid.isEmpty())
		return;

	if (m_networkReply)
	{
		delete m_networkReply;
		m_networkReply = 0;
	}

	m_networkTimer.stop();
	m_uid = uid;
	m_imageBuffer.clear();
	m_imageLength = 0;
	ui->stackedWidget->setCurrentIndex(0);

	GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
	QString urlString = QString("%1/api/pmuser/rawphoto/%2").arg(loginConfig.managerUrl).arg(uid);

	QNetworkRequest request;
	request.setUrl(QUrl(urlString));
	if (HttpPool::needApiCheck())
	{
		// add api check header
		HttpPool::addRequestApiCheck(request);
	}

	m_networkReply = m_networkAccessManager->get(request);
	connect(m_networkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
	connect(m_networkReply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

	m_networkTimer.start();
}

void BigAvatarDialog::refresh()
{
	QString uid = m_uid;
	getBigAvatar(uid);
}

void BigAvatarDialog::onReadyRead()
{
	if (m_imageBuffer.isEmpty())
	{
		m_imageLength = m_networkReply->header(QNetworkRequest::ContentLengthHeader).toInt();
	}

	QByteArray content = m_networkReply->readAll();
	m_imageBuffer.append(content);
}

void BigAvatarDialog::onReplyFinished()
{
	m_networkTimer.stop();

	QNetworkReply::NetworkError netError = m_networkReply->error();
	QString netErrorString = m_networkReply->errorString();
	int netCode = m_networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	m_networkReply->deleteLater();
	m_networkReply = 0;

	if (netError == QNetworkReply::ContentNotFoundError)
	{
		// no big avatar, show small avatar
		QPixmap smallAvatar = qPmApp->getModelManager()->getUserAvatar(m_uid);
		setAvatar(smallAvatar);
		return;
	}

	if (netError != QNetworkReply::NoError)
	{
		// has error
		qWarning() << Q_FUNC_INFO << "http error: " << netCode << netErrorString;
		ui->labelErrorTip->setText(tr("Download error: %1").arg(netCode));
		ui->stackedWidget->setCurrentIndex(1);
		return;
	}

	if (m_imageLength != m_imageBuffer.length())
	{
		// has error
		ui->labelErrorTip->setText(tr("Download error: image data is invalid"));
		ui->stackedWidget->setCurrentIndex(1);
		return;
	}

	QPixmap pixmap;
	if (!pixmap.loadFromData(m_imageBuffer))
	{
		// has error
		ui->labelErrorTip->setText(tr("Download error: image data error"));
		ui->stackedWidget->setCurrentIndex(1);
		return;
	}

	setAvatar(pixmap);

	// save to file
	QImage newImage = pixmap.toImage();
	qPmApp->getDetailPhotoManager()->setAvatar(m_uid, newImage);
}

void BigAvatarDialog::onReplyTimeout()
{
	m_networkTimer.stop();

	if (m_networkReply)
	{
		delete m_networkReply;
		m_networkReply = 0;
	}

	ui->labelErrorTip->setText(tr("Download error: timeout"));
	ui->stackedWidget->setCurrentIndex(1);
	return;
}

void BigAvatarDialog::initUI()
{
	ui->stackedWidget->setCurrentIndex(0);

	QMovie *movie = new QMovie(this);
	movie->setFileName(":/images/loading_image_small.gif");
	ui->labelLoading->setMovie(movie);
	movie->start();

	ui->labelRefresh->setFontAtt(QColor(0, 120, 216), 10, false);
}

void BigAvatarDialog::setAvatar(const QPixmap &pixmap)
{
	int avatarSize = pixmap.width();
	QPixmap avatar = pixmap;

	if (avatarSize < kInitImageSize)
		avatarSize = kInitImageSize;

	QRect desktopRect = QApplication::desktop()->availableGeometry();
	const int kSpacing = 48;
	if (avatarSize > desktopRect.width()-kSpacing || avatarSize > desktopRect.height()-kSpacing)
	{
		avatarSize = (desktopRect.width() < desktopRect.height() ? desktopRect.width()-kSpacing : desktopRect.height()-kSpacing);
		avatar = pixmap.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	QSize size = this->size();
	int moveX = (size.width() - (avatarSize+2*kBorder))/2;
	int moveY = (size.height() - (kTitleBarHeight+avatarSize+2*kBorder))/2;
	QPoint pt = this->pos();
	this->resize(avatarSize+2*kBorder, kTitleBarHeight+avatarSize+2*kBorder);
	pt.setX(pt.x()+moveX);
	pt.setY(pt.y()+moveY);
	if (pt.x() < desktopRect.left())
		pt.setX(desktopRect.left());
	if (pt.y() < desktopRect.top())
		pt.setY(desktopRect.top());
	this->move(pt);

	ui->labelImage->setPixmap(avatar);
	ui->stackedWidget->setCurrentIndex(2);
}
