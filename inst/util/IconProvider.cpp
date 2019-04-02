#include "IconProvider.h"
#include <QFileIconProvider>
#include <QApplication>
#include <QStyle>
#include <QtWin>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <shellapi.h>
#endif // Q_OS_WIN

QIcon IconProvider::fileIcon(const QString &filename)
{
    QFileInfo fileInfo(filename);
    QPixmap pixmap;
	QFileIconProvider iconProvider;

#ifdef Q_OS_WIN

    if (fileInfo.suffix().isEmpty() || fileInfo.suffix() == "exe" && fileInfo.exists())
    {
		return iconProvider.icon(fileInfo);
    }

    SHFILEINFO shFileInfo;
    unsigned long val = 0;

    val = SHGetFileInfoA(("file."+fileInfo.suffix()).toUtf8().constData(), 0, &shFileInfo,
                        sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

    // Even if GetFileInfo returns a valid result, hIcon can be empty in some cases
    if (val && shFileInfo.hIcon)
    {
        pixmap = QtWin::fromHICON(shFileInfo.hIcon);
        DestroyIcon(shFileInfo.hIcon);
    }
    else
    {
		return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }

#else
    // Default icon for Linux and Mac OS X for now
    return iconProvider.icon(fileInfo);
#endif

    return QIcon(pixmap);
}

QIcon IconProvider::dirIcon()
{
	QFileIconProvider iconProvider;
    return iconProvider.icon(QFileIconProvider::Folder);
}