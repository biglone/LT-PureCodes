#ifndef _AVATARUTIL_H_
#define _AVATARUTIL_H_
#include <QPixmap>
#include <QImage>

class AvatarUtil
{
public:
	static QString save(const QString& path, const QString& id, const QString& base64code, const QSize& maxSize);
	static QString save(const QString& path, const QString& id, const QByteArray& fileData, const QSize& maxSize);
	static QString save(const QString& path, const QString& id, const QImage& image, const QSize& maxSize);
	static QString avatarName(const QString& id);
	static QPixmap templateImage(const QString& background, const QImage& img);
};

#endif //_AVATARUTIL_H_
