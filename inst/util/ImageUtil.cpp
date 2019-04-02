#include "ImageUtil.h"
#include <QImage>
#include <QFile>
#include <QDir>
#include <QDebug>

QImage ImageUtil::readImage(const QString &filePath, bool *fromData /*= 0*/)
{
	QImage image;

	if (fromData)
	{
		*fromData = false;
	}

	if (filePath.isEmpty())
	{
		qWarning("ImageUtil::readImage: file path is empty.");
		return image;
	}

	if (!QFile::exists(filePath))
	{
		qWarning("ImageUtil::readImage: file path doesn't exist.");
		return image;
	}

	if (image.load(filePath))
	{
		return image;
	}
	else
	{
		qWarning("ImageUtil::readImage: load image file failed: %s\ntry to load with image data", 
			filePath.toUtf8().constData());
	}

	QFile imageFile(filePath);
	if (imageFile.open(QIODevice::ReadOnly))
	{
		if (!image.loadFromData(imageFile.readAll()))
		{
			qWarning("ImageUtil::readImage: load image data failed: %s", filePath.toUtf8().constData());
		}
		imageFile.close();

		if (fromData)
		{
			*fromData = true;
		}
	}

	return image;
}

bool ImageUtil::saveImage(const QImage &image, const QString &imagePath)
{
	bool ret = false;
	if (image.isNull())
		return ret;

	QFileInfo fileInfo(imagePath);
	QDir dir = fileInfo.absoluteDir();
	if (!dir.exists())
		return ret;

	ret = image.save(imagePath);
	return ret;
}

bool ImageUtil::saveImage(const QString &imagePath, const QString &newPath)
{
	bool ret = false;
	QImage img = ImageUtil::readImage(imagePath);
	if (img.isNull())
	{
		qWarning() << Q_FUNC_INFO << "original image is null: " << imagePath;
		return ret;
	}

	QFileInfo fi(imagePath);
	QString suffixStr = fi.suffix();

	QFileInfo newFi(newPath);
	QString newSuffixStr = newFi.suffix();
	suffixStr = suffixStr.toLower();
	newSuffixStr = newSuffixStr.toLower();
	if (suffixStr == newSuffixStr)
	{
		QFile sourceFile(imagePath);
		QFile destinationFile(newPath);
		if (!destinationFile.exists() || destinationFile.remove()) 
		{
			if (!sourceFile.copy(destinationFile.fileName()))
			{
				// Could not copy file
				qWarning() << Q_FUNC_INFO << " copy file failed: " << imagePath << newPath;
			}
			else
			{
				ret = true;
			}
		}
		else
		{
			// Could not remove file
			qWarning() << Q_FUNC_INFO << " remove file failed: " << imagePath << newPath;
		}
	}
	else
	{
		// save to other format
		if (!img.save(newPath))
		{
			qWarning() << Q_FUNC_INFO << " save file failed: " << newPath;
		}
		else
		{
			ret = true;
		}
	}

	return ret;
}