#include "FileDialog.h"
#include <QDir>
#include <QDebug>
#ifdef Q_OS_WIN
#include <windows.h>
#include <CommDlg.h>
#endif // Q_OS_WIN

QString FileDialog::getOpenFileName(QWidget *parent /*= 0*/, 
									const QString &caption /*= QString()*/, 
									const QString &dir /*= QString()*/, 
									const wchar_t *filter /*= 0*/)
{
	QString fileName;
	OPENFILENAMEW ofn;         // common dialog box structure
	wchar_t szFile[260];       // buffer for file name
	wchar_t szCaption[64];     // buffer for caption
	wchar_t szDir[260];        // buffer for dir
	int len = 0;

	// Initialize OPENFILENAME
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	if (parent)
		ofn.hwndOwner = (HWND)parent->winId();
	ofn.lpstrFile = szFile;

	memset(&szCaption, 0, sizeof(szCaption));
	if (!caption.isEmpty())
	{
		len = caption.toWCharArray(szCaption);
		szCaption[len] = (wchar_t)('\0');
	}

	memset(&szDir, 0, sizeof(szDir));
	if (!dir.isEmpty())
	{
		QString localDir = QDir::toNativeSeparators(dir);
		len = localDir.toWCharArray(szDir);
		if (szDir[len-1] != L'\\')
			szDir[len++] = L'\\';
		szDir[len] = (wchar_t)('\0');
	}

	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = (wchar_t)('\0');
	ofn.nMaxFile = sizeof(szFile)/sizeof(wchar_t);
	ofn.lpstrFilter = filter;
	if (!filter)
		ofn.nFilterIndex = 1;
	ofn.lpstrTitle = szCaption;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = szDir;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS | OFN_NOCHANGEDIR;

	// Display the Open dialog box. 
	if (GetOpenFileNameW(&ofn))
	{
		fileName = QString::fromWCharArray(ofn.lpstrFile);
	}
	return fileName;
}

QStringList FileDialog::getOpenFileNames(QWidget *parent /*= 0*/,
										 const QString &caption /*= QString()*/, 
										 const QString &dir /*= QString()*/, 
										 const wchar_t *filter /*= 0*/)
{
	QStringList fileNames;
	OPENFILENAMEW ofn;         // common dialog box structure
	wchar_t szFile[2048];      // buffer for file name
	wchar_t szCaption[64];     // buffer for caption
	wchar_t szDir[260];        // buffer for dir
	int len = 0;

	// Initialize OPENFILENAME
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	if (parent)
		ofn.hwndOwner = (HWND)parent->winId();
	ofn.lpstrFile = szFile;

	memset(&szCaption, 0, sizeof(szCaption));
	if (!caption.isEmpty())
	{
		len = caption.toWCharArray(szCaption);
		szCaption[len] = (wchar_t)('\0');
	}

	memset(&szDir, 0, sizeof(szDir));
	if (!dir.isEmpty())
	{
		QString localDir = QDir::toNativeSeparators(dir);
		len = localDir.toWCharArray(szDir);
		if (szDir[len-1] != L'\\')
			szDir[len++] = L'\\';
		szDir[len] = (wchar_t)('\0');
	}

	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = (wchar_t)('\0');
	ofn.nMaxFile = sizeof(szFile)/sizeof(wchar_t);
	ofn.lpstrFilter = filter;
	if (!filter)
		ofn.nFilterIndex = 1;
	ofn.lpstrTitle = szCaption;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = szDir;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_NODEREFERENCELINKS | OFN_NOCHANGEDIR;

	// Display the Open dialog box. 
	if (GetOpenFileNameW(&ofn))
	{
		// get file path
		wchar_t tmpBuffer[260];
		memset(tmpBuffer, 0, 260*sizeof(wchar_t));
		memcpy(&tmpBuffer, ofn.lpstrFile, ofn.nFileOffset*sizeof(wchar_t));
		if (tmpBuffer[ofn.nFileOffset-1] == L'\\')
			tmpBuffer[ofn.nFileOffset-1] = L'\0';
		QString filePath = QString::fromWCharArray(tmpBuffer);

		// get all file names
		wchar_t *pBegin = ofn.lpstrFile + ofn.nFileOffset;
		wchar_t *pTmp = pBegin;
		while (true)
		{
			if (*pTmp == '\0')
			{
				memset(tmpBuffer, 0, 260*sizeof(wchar_t));
				memcpy(&tmpBuffer, pBegin, (pTmp-pBegin)*sizeof(wchar_t));
				QString fileName = QString::fromWCharArray(tmpBuffer);
				QString fileFullPath = filePath + "\\" + fileName;
				fileNames.append(fileFullPath);

				pBegin = pTmp + 1;
				if (*pBegin == '\0')
					break;
			}
			++pTmp;
		}
	}
	return fileNames;
}

QString FileDialog::getSaveFileName(QWidget *parent /*= 0*/,
						            const QString &caption /*= QString()*/,
						            const QString &dir /*= QString()*/,
						            const wchar_t *filter /*= 0*/)
{
	QString fileName;
	OPENFILENAMEW ofn;         // common dialog box structure
	wchar_t szFile[260];       // buffer for file name
	wchar_t szInitDir[260];    // init dir
	wchar_t szCaption[64];     // buffer for caption
	int len = 0;

	// Initialize OPENFILENAME
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	if (parent)
		ofn.hwndOwner = (HWND)parent->winId();

	memset(&szCaption, 0, sizeof(szCaption));
	if (!caption.isEmpty())
	{
		len = caption.toWCharArray(szCaption);
		szCaption[len] = (wchar_t)('\0');
	}

	memset(&szFile, 0, sizeof(szFile));
	memset(&szInitDir, 0, sizeof(szInitDir));
	QString originSuffix;
	if (!dir.isEmpty())
	{
		QFileInfo fi(dir);
		QString absPath = QDir::toNativeSeparators(fi.absolutePath());
		len = absPath.toWCharArray(szInitDir);
		szInitDir[len] = (wchar_t)('\0');

		QString initFileName = fi.fileName();
		originSuffix = fi.suffix();
		if (!initFileName.isEmpty())
		{
			len = initFileName.toWCharArray(szFile);
			szFile[len] = (wchar_t)('\0');
		}
	}

	ofn.lpstrInitialDir = szInitDir;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile)/sizeof(wchar_t);
	ofn.lpstrFilter = filter;
	if (!filter)
		ofn.nFilterIndex = 1;
	ofn.lpstrTitle = szCaption;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = 0;
	ofn.Flags = OFN_EXPLORER | OFN_SHOWHELP | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR; 

	// Display the Open dialog box. 
	if (GetSaveFileNameW(&ofn))
	{
		fileName = QString::fromWCharArray(ofn.lpstrFile);
		QFileInfo fi(fileName);
		if (!ofn.lpstrFilter)
		{
			if (fi.suffix().isEmpty() && !originSuffix.isEmpty())
				fileName = QString("%1.%2").arg(fileName).arg(originSuffix);
		}
	}
	else
	{
		unsigned int errCode = CommDlgExtendedError();
		qWarning() << Q_FUNC_INFO << "getSaveFileName canceled or failed: " << errCode << QString::fromWCharArray(szFile);
	}
	return fileName;
}

QString FileDialog::getImageSaveFileName(QWidget *parent /*= 0*/,
										 const QString &caption /*= QString()*/,
										 const QString &imagePath /*= QString()*/)
{
	QStringList filters;
	filters.append(QObject::tr("Image file(*.jpg)"));
	filters.append(QObject::tr("jpg"));
	filters.append(QObject::tr("Image file(*.png)"));
	filters.append(QObject::tr("png"));
	unsigned int defaultFilterIndex = 1;
	QString defaultFilter = filters[1]; // jpg
	if (!imagePath.isEmpty())
	{
		QFileInfo fi(imagePath);
		QString suffix = fi.suffix();
		suffix = suffix.toLower();
		if (!suffix.isEmpty())
		{
			if (suffix.compare("gif") == 0)
			{
				filters.insert(0, QObject::tr("Image file(*.gif)"));
				filters.insert(1, QObject::tr("gif"));
			}

			if (filters.contains(suffix))
			{
				defaultFilter = suffix;
				defaultFilterIndex = filters.indexOf(suffix);
				defaultFilterIndex = (defaultFilterIndex+1)/2;
			}
		}
	}
	
	QString fileName;
	OPENFILENAMEW ofn;         // common dialog box structure
	wchar_t szFile[260];       // buffer for file name
	wchar_t szInitDir[260];    // init dir
	wchar_t szFilter[260];     // filter
	wchar_t szCaption[64];     // buffer for caption
	wchar_t szDefExt[32];      // default ext 
	int len = 0;

	// Initialize OPENFILENAME
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	if (parent)
		ofn.hwndOwner = (HWND)parent->winId();

	memset(&szCaption, 0, sizeof(szCaption));
	if (!caption.isEmpty())
	{
		len = caption.toWCharArray(szCaption);
		szCaption[len] = (wchar_t)('\0');
	}

	memset(&szFile, 0, sizeof(szFile));
	memset(&szInitDir, 0, sizeof(szInitDir));
	memset(&szFilter, 0, sizeof(szFilter));

	QString dir = imagePath;
	if (!dir.isEmpty())
	{
		QFileInfo fi(dir);
		QString absPath = QDir::toNativeSeparators(fi.absolutePath());
		len = absPath.toWCharArray(szInitDir);
		szInitDir[len] = (wchar_t)('\0');

		QString initFileName = fi.completeBaseName() + "." + defaultFilter;
		if (!initFileName.isEmpty())
		{
			len = initFileName.toWCharArray(szFile);
			szFile[len] = (wchar_t)('\0');
		}
	}

	wchar_t *pFilter = szFilter;
	for (int i = 0; i < filters.count(); i++)
	{
		QString filterStr = filters[i];
		if (i%2 != 0)
		{
			filterStr.prepend("*.");
		}
		filterStr.toWCharArray(pFilter);
		*(pFilter+filterStr.length()) = (wchar_t)('\0');
		pFilter += (filterStr.length()+1);
	}
	*pFilter = (wchar_t)('\0');

	defaultFilter.toWCharArray(szDefExt);
	szDefExt[defaultFilter.length()] = (wchar_t)('\0');


	ofn.lpstrInitialDir = szInitDir;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile)/sizeof(wchar_t);
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = defaultFilterIndex;
	ofn.lpstrTitle = szCaption;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrDefExt = szDefExt;
	ofn.Flags = OFN_EXPLORER | OFN_SHOWHELP | OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT | OFN_NOCHANGEDIR; 

	// Display the Open dialog box. 
	if (GetSaveFileNameW(&ofn))
	{
		fileName = QString::fromWCharArray(ofn.lpstrFile);
	}
	else
	{
		unsigned int errCode = CommDlgExtendedError();
		qWarning() << Q_FUNC_INFO << "getSaveFileName canceled or failed: " << errCode << QString::fromWCharArray(szFile);
	}

	return fileName;
}