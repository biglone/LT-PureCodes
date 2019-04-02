#include <QDebug>
#include <QByteArray>
#include <QDir>
#include <QDateTime>
#include <QApplication>
#include <QPainter>
#include <QStyle>

#include "AvatarUtil.h"

QString AvatarUtil::save(const QString& path, const QString& id, const QString& base64code, const QSize& maxSize)
{
	QByteArray ba = QByteArray::fromBase64(base64code.toLatin1());
	return AvatarUtil::save(path, id, ba, maxSize);
}

QString AvatarUtil::save(const QString& path, const QString& id, const QByteArray& fileData, const QSize& maxSize)
{
	QString fileName;
	QImage img = QImage::fromData(fileData);
	if (img.isNull())
		return fileName;

	return AvatarUtil::save(path, id, img, maxSize);
}

QString AvatarUtil::save(const QString& path, const QString& id, const QImage& image, const QSize& maxSize)
{
	QString sFName;
	if (image.isNull())
		return sFName;

	QDir dir(path);

	// delete
	QStringList filter;
	filter << QString("%1_*.avatar").arg(id);
	filter << QString("%1.avatar").arg(id);
	foreach (QString file, dir.entryList(filter, QDir::Files))
	{
		dir.remove(file);
	}

	sFName = AvatarUtil::avatarName(id);

	QImage saveImage = image;
	if (image.width() > maxSize.width() || image.height() > maxSize.height())
	{
		saveImage = image.scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	bool bRet = saveImage.save(dir.absoluteFilePath(sFName), "jpg");

	qWarning() << Q_FUNC_INFO << dir.absoluteFilePath(sFName) << sFName << bRet;

	return sFName;
}

QString AvatarUtil::avatarName(const QString& id)
{
	return QString("%1.avatar").arg(id);
}

QPixmap AvatarUtil::templateImage(const QString& background, const QImage& img)
{
	QPixmap ret(180, 180);

	// background
	ret.fill(QColor(background));

	QPainter paint(&ret);
	QRect rect(0,0, ret.width(), ret.height());

	QPixmap pix = QPixmap::fromImage(img.scaled(ret.size(), Qt::KeepAspectRatio));

	paint.save();
	paint.setClipRect(rect);
	qApp->style()->drawItemPixmap(&paint, rect, Qt::AlignCenter, pix);
	paint.restore();

	return ret;
}

