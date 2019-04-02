#include "SnapShot.h"
#include "qxttimer.h"
#include "Account.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDebug>
#include "ScreenShotDlg.h"

SnapShot::SnapShot(QObject *parent) : QObject(parent), m_snapshoting(false)
{
	m_screenShotDlg.reset(new ScreenShotDlg());
	connect(m_screenShotDlg.data(), SIGNAL(screenShotDone(QPixmap)), this, SLOT(onSnapshotOK(QPixmap)));
	connect(m_screenShotDlg.data(), SIGNAL(screenShotCancelled()), this, SLOT(onSnapshotCancelled()));
}

SnapShot::~SnapShot()
{
}

void SnapShot::shot()
{
	if (m_snapshoting)
		return;

	m_screenShotDlg->startScreenShot();
	m_snapshoting = true;
}

void SnapShot::onSnapshotOK(const QPixmap &pixmap)
{
	// save to file
	QString imagePath = Account::instance()->imagePath();
	QString imageFilePath = QString("%1/%2.png").arg(imagePath)
		.arg(QDateTime::currentDateTime()
		.toString("yyyyMMddhhmmss"));
	pixmap.save(imageFilePath, "png");
	qWarning() << "screen shot is saved to " << imageFilePath;

	// set to clipboard
	QApplication::clipboard()->clear();
	QApplication::clipboard()->setPixmap(pixmap);

	// send snapshot ok signal
	QString localImageFilePath = QDir::toNativeSeparators(imageFilePath);
	QxtTimer::singleShot(50, this, SIGNAL(snapShotted(QString)), localImageFilePath);
	m_snapshoting = false;
}

void SnapShot::onSnapshotCancelled()
{
	QTimer::singleShot(50, this, SIGNAL(snapShotCancelled()));
	m_snapshoting = false;
}
