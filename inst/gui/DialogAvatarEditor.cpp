#include "DialogAvatarEditor.h"
#include "ui_DialogAvatarEditor.h"
#include <QStandardPaths>
#include <math.h>
#include "util/FileDialog.h"
#include "pmessagebox.h"
#include "takephotodialog.h"
#include "PmApp.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "settings/GlobalSettings.h"
#include <QBuffer>
#include <QHttpMultiPart>
#include <QDateTime>
#include "bigavatardialog.h"
#include "widgetmanager.h"
#include "loginmgr.h"
#include "util/ImageUtil.h"
#include "http/HttpPool.h"

DialogAvatarEditor::DialogAvatarEditor(const QString &uid, QWidget *parent /* = 0 */) :
    FramelessDialog(parent),
    ui(new Ui::DialogAvatarEditor),
	m_uid(uid)
{
    ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->labelTitle->text());

    setMainLayout(ui->verticalLayoutMain);

	resize(583, 448);
	setFixedSize(583, 448);
	setMaximizeable(false);
	setResizeable(false);

	m_bModified = false;

    connect(ui->pushButtonBrowse, SIGNAL(clicked()), SLOT(browse()));
	connect(ui->pushButtonPhoto, SIGNAL(clicked()), SLOT(photo()));
	connect(ui->pushButtonEdit, SIGNAL(clicked()), SLOT(edit()));
    connect(ui->toolButtonZoomIn, SIGNAL(clicked()), SLOT(zoomIn()));
    connect(ui->toolButtonZoomOut, SIGNAL(clicked()), SLOT(zoomOut()));
    connect(ui->toolButtonZoomFit, SIGNAL(clicked()), ui->graphicsView,  SLOT(resetView()));
    connect(ui->toolButtonOriginal, SIGNAL(clicked()), ui->graphicsView, SLOT(showOriginalPixmap()));
    connect(ui->toolButtonRotate, SIGNAL(clicked()), ui->graphicsView, SLOT(rotatePixmap()));
    connect(ui->graphicsView, SIGNAL(selectedPixmapChanged(QPixmap)), SLOT(selectedPixmapChanged(QPixmap)));

    ui->toolButtonOriginal->setEnabled(false);
    ui->toolButtonZoomIn->setEnabled(false);
    ui->toolButtonZoomOut->setEnabled(false);
    ui->toolButtonZoomFit->setEnabled(false);
    ui->toolButtonRotate->setEnabled(false);

	connect(ui->pushButtonOK, SIGNAL(clicked()), SLOT(onPushButtonOKClicked()));
	connect(ui->pushButtonCancel, SIGNAL(clicked()), SLOT(reject()));
	connect(ui->btnClose, SIGNAL(clicked()), SLOT(reject()));
	connect(ui->btnMinimize, SIGNAL(clicked()), SLOT(showMinimized()));
    ui->graphicsView->setSceneRect(QRect(0, 0, ui->graphicsView->width(), ui->graphicsView->height()));
    ui->graphicsView->resetView();

	m_networkAccessManager = new QNetworkAccessManager(this);
	connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(onUpdateFinished(QNetworkReply *)));

	connect(ui->labelViewBigAvatar, SIGNAL(clicked()), this, SLOT(viewBigAvatar()));
	
	setSkin();
}

DialogAvatarEditor::~DialogAvatarEditor()
{
    delete ui;
}

void DialogAvatarEditor::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_8.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title style sheet 
	ui->labelTitle->setStyleSheet("QLabel{font-size: 12pt; color: white;}");

	// ui style
	ui->pushButtonEdit->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelViewBigAvatar->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelTipSmall->setStyleSheet("font: 9pt;");
	ui->labelTipBig->setStyleSheet("font: 9pt;");
	ui->previewWidget->setStyleSheet("QWidget#previewWidget {border-left: 1px solid rgb(229, 229, 229); background: white;}");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->pushButtonBrowse->setStyleSheet(qss);
		ui->pushButtonPhoto->setStyleSheet(qss);
		ui->pushButtonOK->setStyleSheet(qss);
		ui->pushButtonCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void DialogAvatarEditor::onPushButtonOKClicked()
{
	if (isModified() && !m_avatar.isNull())
	{
		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::information(this, tr("Tip"), tr("You are offline, please change avatar when online"));
			return;
		}

		QBuffer *buf = new QBuffer();
		if (!m_avatar.save(buf, "jpg"))
		{
			qWarning() << Q_FUNC_INFO << "save image buffer failed";
			delete buf;
			buf = 0;
			return;
		}

		// update avatar
		ui->widgetCentral->setEnabled(false);
		setCursor(Qt::WaitCursor);

		QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
		multiPart->setBoundary("---------------------------7dc37b1860260");

		QHttpPart textPart;
		textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userId\""));
		textPart.setBody(m_uid.toUtf8());

		QHttpPart imagePart;
		imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
		imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, 
			QVariant(QString("form-data; name=\"photo\"; filename=\"%1.jpg\"").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))));
		buf->open(QIODevice::ReadOnly);
		imagePart.setBodyDevice(buf);
		buf->setParent(multiPart);

		multiPart->append(textPart);
		multiPart->append(imagePart);

		GlobalSettings::LoginConfig loginConfig = GlobalSettings::curLoginConfig();
		QString urlString = QString("%1/api/pmuser/uploadphoto").arg(loginConfig.managerUrl);

		QNetworkRequest request;
		request.setUrl(QUrl(urlString));
		if (HttpPool::needApiCheck())
		{
			// add api check header
			HttpPool::addRequestApiCheck(request);
		}

		QNetworkReply *reply = m_networkAccessManager->post(request, multiPart);
		multiPart->setParent(reply);
	}
	else
	{
		reject();
	}
}

void DialogAvatarEditor::onUpdateFinished(QNetworkReply *reply)
{
 	setCursor(Qt::ArrowCursor);
	ui->widgetCentral->setEnabled(true);

	QNetworkReply::NetworkError netError = reply->error();
	QString netErrorString = reply->errorString();
	int netCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	reply->deleteLater();
	reply = 0;

	if (netError != QNetworkReply::NoError)
	{
		qWarning() << Q_FUNC_INFO << "http error: " << netCode << netErrorString;
		PMessageBox::warning(this, tr("Error"), tr("Uploading avatar failed: %1").arg(netCode));
		return;
	}

	PMessageBox::information(this, tr("Tip"), tr("Uploading avatar succeeded"));
	accept();
	return;
}

void DialogAvatarEditor::viewBigAvatar()
{
	BigAvatarDialog *bigAvatarDialog = BigAvatarDialog::getDialog();
	bigAvatarDialog->getBigAvatar(m_uid);
	WidgetManager::showActivateRaiseWindow(bigAvatarDialog);
}

void DialogAvatarEditor::selectedPixmapChanged(const QPixmap &pixmap)
{
	const int kMinAvatarSize = 180;
	const int kMaxAvatarSize = 640;
	if (pixmap.width() < kMinAvatarSize)
	{
		m_avatar = pixmap.scaled(QSize(kMinAvatarSize, kMinAvatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else if (pixmap.width() > kMaxAvatarSize)
	{
		m_avatar = pixmap.scaled(QSize(kMaxAvatarSize, kMaxAvatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else
	{
		m_avatar = pixmap;
	}
    ui->labelAvatarBig->setPixmap(m_avatar);
	ui->labelAvatarSmall->setPixmap(m_avatar);
	m_bModified = true;
}

void DialogAvatarEditor::setOrignalAvatar(const QPixmap &pixmap)
{
    ui->labelAvatarBig->setPixmap(pixmap);
	ui->labelAvatarSmall->setPixmap(pixmap);
    m_avatar = pixmap;
	m_bModified = false;
}

QPixmap DialogAvatarEditor::getAvatar()
{
    return m_avatar;
}

void DialogAvatarEditor::browse()
{
	// filter
	QString filterTag = tr("Image Files");
	wchar_t szFilter[256];
	memset(szFilter, 0, sizeof(szFilter));
	int len = filterTag.toWCharArray(szFilter);
	wchar_t szFilterSuffix[] = L"(*.bmp;*.jpg;*.jpeg;*.png)\0*.BMP;*.JPG;*.JPEG;*.PNG\0\0";
	memcpy((char *)(szFilter+len), (char *)szFilterSuffix, sizeof(szFilterSuffix));

    QString fileName = FileDialog::getOpenFileName(this, tr("Choose Image"),
                                                   QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                   szFilter);
    if(!fileName.isEmpty())
    {
		QFile file(fileName);
		if (file.size() > 5120*1024) // max 5M
		{
			PMessageBox::information(this, tr("Tip"), tr("Please choose image less than 5MB"));
			return;
		}

		QImage image = ImageUtil::readImage(fileName);
		QPixmap pixmap = QPixmap::fromImage(image);
		if (!pixmap.isNull())
		{
			setPixmap(pixmap);
		}
    }
}

void DialogAvatarEditor::photo()
{
	if (qPmApp->hasVideoSession())
	{
		PMessageBox::information(this, tr("Tip"), tr("You are in a video session, can't take photo"));
		return;
	}

	TakePhotoDialog dlg(this);
	if (!dlg.startCamera())
	{
		PMessageBox::warning(this, tr("Tip"), tr("Camera failed, please check if there is a camera or it is used by another program"));
		return;
	}

	qPmApp->setTakeSelfPhoto(true);

	dlg.setWindowModality(Qt::WindowModal);
	if (dlg.exec())
	{
		QPixmap pixmap = dlg.photo();
		if (!pixmap.isNull())
		{
			setPixmap(pixmap);
		}
	}

	qPmApp->setTakeSelfPhoto(false);
}

void DialogAvatarEditor::edit()
{
	if (!m_avatar.isNull())
	{
		setPixmap(m_avatar);
	}
}

void DialogAvatarEditor::zoomIn()
{
    ui->graphicsView->scaleView(1/0.8);
}

void DialogAvatarEditor::zoomOut()
{
    ui->graphicsView->scaleView(0.8);
}

void DialogAvatarEditor::setPixmap(const QPixmap& pixmap)
{
	ui->toolButtonOriginal->setEnabled(true);
	ui->toolButtonZoomIn->setEnabled(true);
	ui->toolButtonZoomOut->setEnabled(true);
	ui->toolButtonZoomFit->setEnabled(true);
	ui->toolButtonRotate->setEnabled(true);
	ui->graphicsView->setPixmap(pixmap);
	m_bModified = true;
}



