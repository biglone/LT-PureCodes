#ifndef __IMAGE_UTIL_H__
#define __IMAGE_UTIL_H__

#include <QString>
#include <QFont>

class ImageUtil
{
public:
	static QImage readImage(const QString &filePath, bool *fromData = 0);

	static bool saveImage(const QImage &image, const QString &imagePath);

	static bool saveImage(const QString &imagePath, const QString &newPath);
};

#endif // __IMAGE_UTIL_H__