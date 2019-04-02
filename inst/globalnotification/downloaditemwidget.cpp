#include "downloaditemwidget.h"
#include <QPainter>
#include <QPen>
#include <QCursor>
#include <QNetworkReply>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QFileIconProvider>
#include <QDebug>
#include <QDir>
#include <math.h>
#include "pmessagebox.h"
#include "util/FileUtil.h"
#include <QProcess>
#include <QDesktopServices>
#include "util/ExplorerUtil.h"

#ifdef Q_OS_WIN
#ifndef UNICODE
#define UNICODE
#endif
#endif // Q_OS_WIN

DownloadItemWidget::DownloadItemWidget(const QString saveFileName, QNetworkReply *reply, 
									   QNetworkAccessManager *nam, bool autoOpen /* = false */, QWidget *parent /* = 0 */)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	initUI();

	m_flashIndex = 0;
	m_flashTimer.setInterval(400);
	m_flashTimer.setSingleShot(false);
	connect(&m_flashTimer, SIGNAL(timeout()), this, SLOT(flashTimeout()));

	m_reply = reply;
	m_bAutoOpen = autoOpen;
	m_nam = nam;
	m_output.setFileName(saveFileName);
	QFileInfo fi(saveFileName);
	ui.labelFileName->setText(fi.fileName());
	m_bytesReceived = 0;
	m_lastBytesReceived = 0;
	m_bytesTotal = -1;
	m_status = Status_Stoped;
	m_url = m_reply->url();

	connect(ui.pushButtonCancel, SIGNAL(clicked()), this, SLOT(stop()));
	connect(ui.pushButtonRetry, SIGNAL(clicked()), this, SLOT(tryAgain()));
	connect(ui.pushButtonOpenFile, SIGNAL(clicked()), this, SLOT(open()));
	connect(ui.pushButtonOpenDir, SIGNAL(clicked()), this, SLOT(openFolder()));

	init();
}

DownloadItemWidget::~DownloadItemWidget()
{

}

bool DownloadItemWidget::isChecked()
{
	return ui.checkBoxCheck->isChecked();
}

void DownloadItemWidget::setChecked(bool checked)
{
	ui.checkBoxCheck->setChecked(checked);
}

void DownloadItemWidget::flash()
{
	m_flashTimer.stop();
	m_flashIndex = 0;
	m_flashTimer.start();
}

void DownloadItemWidget::paintEvent(QPaintEvent * /*e*/)
{
	QPainter painter(this);
	QRect frameRect = rect();

	QPen pen;
	pen.setWidth(1);
	pen.setColor(QColor(209, 209, 209));
	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);
	QPoint ptLeft = frameRect.bottomLeft();
	ptLeft.setX(ptLeft.x()+20);
	QPoint ptRight = frameRect.bottomRight();
	ptRight.setX(ptRight.x()-20);
	painter.drawLine(ptLeft, ptRight);

	if (m_status == Status_Downloading)
	{
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(156,236,107));
		QRect processRect = frameRect.adjusted(18, 1, -18, -2);
		processRect.setWidth(processRect.width() * m_bytesReceived * 1.0 / m_bytesTotal);
		if (processRect.isValid())
		{
			painter.drawRect(processRect);
		}
	}
	else if (m_flashIndex%2)
	{
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(246,240,115));
		QRect processRect = frameRect.adjusted(18, 1, -18, -2);
		if (processRect.isValid())
		{
			painter.drawRect(processRect);
		}
	}
}

void DownloadItemWidget::stop()
{
	setUpdatesEnabled(false);
	setStatus(Status_Stoped);
	setUpdatesEnabled(true);
	m_reply->abort();
	m_output.close();
}

void DownloadItemWidget::tryAgain()
{
	if (m_status != Status_Stoped && m_status != Status_Failed && m_status != Status_Successfully)
		return;

	m_bytesReceived = 0;
	m_lastBytesReceived = 0;
	m_bytesTotal = -1;

	QNetworkReply *r = m_nam->get(QNetworkRequest(m_url));
	if (m_reply)
		delete m_reply;
	m_reply = r;

	init();
}

void DownloadItemWidget::mouseReleaseEvent(QMouseEvent *e)
{
	/* // do not default open
	if (m_status == Status_Successfully)
	{
		open();
	}
	*/

	return QWidget::mouseReleaseEvent(e);
}


void DownloadItemWidget::init()
{
	if (!m_reply)
		return;

	if (m_output.exists())
		m_output.remove();

	if (!m_output.isOpen()) {
		// in case someone else has already put a file there
		if (!m_output.open(QIODevice::WriteOnly)) {
			qWarning() << Q_FUNC_INFO << "open file failed: " << m_output.errorString();
			ui.labelInfo->setText(tr("Open file failed"));
			setStatus(Status_Failed);
			return;
		}
	}

	// set file icon
	QFileInfo fi(m_output.fileName());
	QString format = fi.suffix().toLower();
	QStringList suffixes;
	suffixes << "doc" << "docx" << "xls" << "xlsx" << "ppt" << "pptx" << "gif" 
		<< "jpeg" << "jpg" << "png" << "bmp" << "pdf" << "swf" << "zip" << "rar" << "txt"
		<< "avi" << "wma" << "rmvb" << "rm" << "mp4" << "3gp";
	if (suffixes.contains(format, Qt::CaseSensitive))
	{
		QString fileName = QString(":/html/images/file_formats/%1.png").arg(format);
		QPixmap pixmap(fileName);
		ui.labelIcon->setPixmap(pixmap);
	}
	else
	{
		QString fileName = QString(":/html/images/file_formats/other.png");
		QPixmap pixmap(fileName);
		ui.labelIcon->setPixmap(pixmap);
	}

	// attach to the m_reply
	m_url = m_reply->url();
	m_reply->setParent(this);

	connect(m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
	connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
		this, SLOT(error(QNetworkReply::NetworkError)));
	connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)),
		this, SLOT(downloadProgress(qint64, qint64)));
	connect(m_reply, SIGNAL(metaDataChanged()),
		this, SLOT(metaDataChanged()));
	connect(m_reply, SIGNAL(finished()),
		this, SLOT(finished()));

	// reset info
	ui.labelInfo->setText(tr("Prepare downloading..."));

	// start timer for the download estimation
	m_downloadTime.start();
	setStatus(Status_Downloading);

	if (m_reply->error() != QNetworkReply::NoError) 
	{
		setStatus(Status_Failed);
		error(m_reply->error());
		finished();
	}
	else
	{
		if (m_status == Status_Successfully)
		{
			finished();
		}
	}
}

void DownloadItemWidget::downloadReadyRead()
{
	if (!m_output.isOpen()) {
		// in case someone else has already put a file there
		if (!m_output.open(QIODevice::WriteOnly)) {
			qWarning() << Q_FUNC_INFO << "open file failed: " << m_output.errorString();
			ui.labelInfo->setText(tr("Open file failed"));
			setStatus(Status_Failed);
			return;
		}
	}

	if (-1 == m_output.write(m_reply->readAll())) {
		qWarning() << Q_FUNC_INFO << "save file failed: " << m_output.errorString();
		ui.labelInfo->setText(tr("Save file failed"));
		setStatus(Status_Failed);
	}
}


void DownloadItemWidget::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	m_lastBytesReceived = m_bytesReceived;
	m_bytesReceived = bytesReceived;
	m_bytesTotal = bytesTotal;
	updateInfoLabel();
	update();
}

void DownloadItemWidget::finished()
{
	if (m_reply->error() == QNetworkReply::NoError) 
	{
		setStatus(Status_Successfully);
	}

	m_output.close();
	
	updateInfoLabel();
	update();

	if (m_status == Status_Successfully)
	{
		if (m_bAutoOpen)
		{
			open();
		}
	}
}

void DownloadItemWidget::flashTimeout()
{
	++m_flashIndex;
	if (m_flashIndex > 7)
	{
		m_flashIndex = 0;
		m_flashTimer.stop();
	}
	update();
}

void DownloadItemWidget::metaDataChanged()
{
    qDebug() << Q_FUNC_INFO << "not handled";
}

void DownloadItemWidget::error(QNetworkReply::NetworkError)
{
	int netCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	qWarning() << Q_FUNC_INFO << "http error: " << netCode << m_reply->errorString() << m_url;
	ui.labelInfo->setText(tr("Network error: %1").arg(netCode));
	
	setStatus(Status_Failed);
}

void DownloadItemWidget::updateInfoLabel()
{
	if (m_reply->error() != QNetworkReply::NoError)
		return;

	// update info label
	double speed = (m_bytesReceived-m_lastBytesReceived) * 1000.0 / m_downloadTime.elapsed();
	double timeRemaining = ((double)(m_bytesTotal - m_bytesReceived)) / speed;
	QString timeRemainingString = tr("s");
	if (timeRemaining > 60) {
		timeRemaining = timeRemaining / 60;
		timeRemainingString = tr("m");
	}
	timeRemaining = floor(timeRemaining);

	// When downloading the eta should never be 0
	if (timeRemaining == 0)
		timeRemaining = 1;

	QString info;
	if (m_status == Status_Downloading) {
		QString remaining;
		if (m_bytesTotal != -1)
			remaining = tr("left %4 %5")
			.arg(timeRemaining)
			.arg(timeRemainingString);
		info = QString(tr("finished %1/%2 (%3/s)  --  %4"))
			.arg(dataString(m_bytesReceived))
			.arg(m_bytesTotal == -1 ? QString("?") : dataString(m_bytesTotal))
			.arg(dataString((int)((int)speed > 0 ? speed : 0)))
			.arg(remaining);
	} else {
		if (m_bytesReceived == m_bytesTotal)
			info = tr("Download finished %1").arg(dataString(m_output.size()));
		else
			info = tr("Download stopped %1/%2")
			.arg(dataString(m_bytesReceived))
			.arg(dataString(m_bytesTotal));
	}
	ui.labelInfo->setText(info);
}

QString DownloadItemWidget::dataString(int size) const
{
	QString str;
	if (size >= (2 << 29))
	{
		str.sprintf("%.1f GB", (double)size/(2<<29));
	}
	else if (size >= (2<<19))
	{
		str.sprintf("%.1f MB", (double)size/(2<<19));
	}
	else if (size >= (2<<9))
	{
		str.sprintf("%.1f KB", (double)size/(2<<9));
	}
	else
	{
		str.sprintf("%d bytes", size);
	}
	return str;
}

void DownloadItemWidget::setStatus(Status s)
{
	m_status = s;

	if (m_status == Status_Downloading)
	{
		ui.pushButtonCancel->setVisible(true);
		ui.pushButtonRetry->setVisible(false);
		ui.pushButtonOpenFile->setVisible(false);
		ui.pushButtonOpenDir->setVisible(false);
	}
	else if (m_status == Status_Stoped)
	{
		ui.pushButtonCancel->setVisible(false);
		ui.pushButtonRetry->setVisible(true);
		ui.pushButtonOpenFile->setVisible(false);
		ui.pushButtonOpenDir->setVisible(false);
	}
	else if (m_status == Status_Failed)
	{
		ui.pushButtonCancel->setVisible(false);
		ui.pushButtonRetry->setVisible(true);
		ui.pushButtonOpenFile->setVisible(false);
		ui.pushButtonOpenDir->setVisible(false);
	}
	else if (m_status == Status_Successfully)
	{
		ui.pushButtonCancel->setVisible(false);
		ui.pushButtonRetry->setVisible(false);
		ui.pushButtonOpenFile->setVisible(true);
		ui.pushButtonOpenDir->setVisible(true);
	}
}


void DownloadItemWidget::openFolder()
{
	QFileInfo fi(m_output);
	QString sPath = fi.absoluteFilePath();
	if (!fi.dir().exists())
	{
		PMessageBox::warning(this, tr("Tip"), tr("This directory does not exist, may be deleted or moved to other place"));
		return;
	}

	ExplorerUtil::selectFile(fi);
}

void DownloadItemWidget::open()
{
	if (m_status != Status_Successfully)
		return;

	QFileInfo info(m_output);
	QString sPath = info.absoluteFilePath();
	if (!FileUtil::fileExists(sPath))
	{
		PMessageBox::warning(this, tr("Tip"), tr("The file does not exist, may be deleted or moved to other place"));
		return;
	}

	QUrl url = QUrl::fromLocalFile(sPath);
	bool bResult = QDesktopServices::openUrl(url);
	if (!bResult)
	{
		PMessageBox::warning(this, tr("Tip"), tr("The file can't be opened, please install related program"));
	}
}

void DownloadItemWidget::initUI()
{
	StylePushButton::Info info;
	info.urlNormal = ":/images/fm_openfile_normal.png";
	info.urlHover = ":/images/fm_openfile_hover.png";
	info.urlPressed = ":/images/fm_openfile_pressed.png";
	info.tooltip = tr("Open File");
	ui.pushButtonOpenFile->setInfo(info);

	info.urlNormal = ":/images/fm_opencatalog_normal.png";
	info.urlHover = ":/images/fm_opencatalog_hover.png";
	info.urlPressed = ":/images/fm_opencatalog_pressed.png";
	info.tooltip = tr("Open Dir");
	ui.pushButtonOpenDir->setInfo(info);

	info.urlNormal = ":/images/fm_delfile_normal.png";
	info.urlHover = ":/images/fm_delfile_hover.png";
	info.urlPressed = ":/images/fm_delfile_pressed.png";
	info.tooltip = tr("Cancel Download");
	ui.pushButtonCancel->setInfo(info);

	info.urlNormal = ":/images/fm_download_normal.png";
	info.urlHover = ":/images/fm_download_hover.png";
	info.urlPressed = ":/images/fm_download_pressed.png";
	info.tooltip = tr("Restart Download");
	ui.pushButtonRetry->setInfo(info);

	ui.labelInfo->setStyleSheet("color: rgb(128, 128, 128);");
}
