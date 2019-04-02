#ifndef _FILEDIALOG_H_
#define _FILEDIALOG_H_

#include <QString>
#include <QStringList>
#include <QWidget>

class FileDialog
{
public:
	static QString getOpenFileName(QWidget *parent = 0, 
		                           const QString &caption = QString(), 
								   const QString &dir = QString(), 
								   const wchar_t *filter = 0);

	static QStringList getOpenFileNames(QWidget *parent = 0,
										const QString &caption = QString(), 
										const QString &dir = QString(), 
										const wchar_t *filter = 0);

	static QString getSaveFileName(QWidget *parent = 0,
		                           const QString &caption = QString(),
								   const QString &dir = QString(),
								   const wchar_t *filter = 0);

	static QString getImageSaveFileName(QWidget *parent = 0,
										const QString &caption = QString(),
										const QString &imagePath = QString());
};

#endif // _FILEDIALOG_H_
