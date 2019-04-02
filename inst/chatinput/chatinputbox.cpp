#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QTextDocument>
#include <QMovie>
#include <QDomDocument>
#include <QDomElement>
#include <QStandardItemModel>
#include <QCompleter>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QClipboard>
#include <QDesktopServices>
#include <QStandardPaths>

#include "application/Logger.h"
using namespace application;

#include "attextobject.h"
#include "filetextobject.h"
#include "chatinputbox.h"
#include "chatinputdef.h"

#include "login/Account.h"
#include "pmessagebox.h"
#include "commoncombobox.h"
#include "util/FileDialog.h"
#include "util/IconProvider.h"
#include "util/ImageUtil.h"
#include "util/PinYinConverter.h"
#include "webview.h"
#include "util/FileUtil.h"
#include "qt-json/json.h"
#include "PmApp.h"
#include "bean/attachitem.h"

static const int PINYIN_ROLE = Qt::UserRole + 1;
static const int ATID_ROLE   = Qt::UserRole + 2;

static const int kMaxDisplayImageHeight = 60;
static const int kMaxDisplayImageWidth  = 248;
static const int kMinDisplayImageHeight = 18;

CChatInputBox::CChatInputBox(QWidget *parent)
	: QTextEdit(parent)
{
	QObject *fileInterface = new CFileTextObject();
	document()->documentLayout()->registerHandler(PM::FileTextFormat, fileInterface);

	QObject *atInterface = new CAtTextObject();
	document()->documentLayout()->registerHandler(PM::AtTextFormat, atInterface);

	ensureCursorVisible();

	initCompleter();

	m_actionCopyImage = new QAction(tr("Copy Image"), this);
	m_actionSaveImage = new QAction(tr("Save Image"), this);

	connect(m_actionCopyImage, SIGNAL(triggered()), SLOT(copyImage()));
	connect(m_actionSaveImage, SIGNAL(triggered()), SLOT(saveImage()));
	connect(this, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

CChatInputBox::~CChatInputBox()
{
	clearResource();
}

void CChatInputBox::setFaceSource(const QStringList& strlstName, const QStringList& strlstSrc)
{
	m_faceNames = strlstName;
	m_faceFileNames = strlstSrc;
}

void CChatInputBox::setCompleteStrings(const QStringList &completeStrings)
{
	QStringList strs = completeStrings;
	
	QString name;
	QString uid;
	m_completerModel->clear();
	foreach (QString str, strs)
	{	
		name = str;
		uid = "";
		int idIndex = str.lastIndexOf("(");
		if (idIndex != -1)
		{
			name = str.left(idIndex);
			uid = str.mid(idIndex+1, str.length()-idIndex-2);
		}

		QStandardItem *item = new QStandardItem(name);
		item->setData(uid, ATID_ROLE);

		QStringList quanPin = PinyinConveter::instance().quanpin(name);
		QString quanPinStr = quanPin.join("");
		item->setData(quanPinStr, PINYIN_ROLE);

		m_completerModel->appendRow(item);
	}
}

void CChatInputBox::insertFiles(const QStringList& rsFiles)
{
	foreach (QString file, rsFiles)
	{
		insertFile(file);
	}
}

void CChatInputBox::insertFile(const QString& rsFile)
{
	Q_ASSERT_X(!rsFile.isEmpty(), "!rsFile.isEmpty", "rsFile.isEmpty");

	if (rsFile.isEmpty())
		return;

	// dir is not supported
	QFileInfo fi(rsFile);
	if (fi.isDir())
		return;

	QString sFilename = fi.fileName();
	sFilename = QApplication::fontMetrics().elidedText(sFilename, Qt::ElideMiddle, 140);

	// empty file is not support
	if (fi.size() == 0)
	{
		emit showInfoTip(tr("The file(%1) you select has no content, can't upload").arg(sFilename));
		return;
	}

	// max file size if 4GB
	if (fi.size() > 0xffffffff)
	{
		emit showInfoTip( tr("The file(%1) you select too large, can't upload").arg(sFilename));
		return;
	}

	if (!canInsertFile(rsFile))
		return;

	QTextCursor cursor = textCursor();

	QTextCharFormat fileCharFormat = cursor.charFormat();
	fileCharFormat.setObjectType(PM::FileTextFormat);

	QIcon icon = IconProvider::fileIcon(rsFile);
	QSize s = icon.actualSize(QSize(16,16));
	QImage img = icon.pixmap(s).toImage();

	fileCharFormat.setProperty(PM::FileIcon, img);
	fileCharFormat.setProperty(PM::FileName, fi.fileName());
	fileCharFormat.setProperty(PM::FilePath, rsFile);
	fileCharFormat.setToolTip(rsFile);
	fileCharFormat.setVerticalAlignment(QTextCharFormat::AlignBaseline);
	cursor.insertText(QString(QChar::ObjectReplacementCharacter), fileCharFormat);
	
	setTextCursor(cursor);
}

void CChatInputBox::insertDir(const QString &rsDir)
{
	QFileInfo dirInfo(rsDir);
	if (!dirInfo.isDir())
		return;

	const int kMaxDirFileNumber = 100;
	const int kMaxDirFileSize   = 500*1024*1024; // 500M

	// check if this dir file number
	QDir dir(rsDir);
	QStringList filePathes = FileUtil::allDirFiles(dir);
	if (filePathes.isEmpty())
	{
		emit showInfoTip(tr("The dir is empty"));
		return;
	}

	if (filePathes.count() > kMaxDirFileNumber)
	{
		emit showInfoTip(tr("The dir file number is no more than %1 files").arg(kMaxDirFileNumber));
		return;
	}

	// check file size in this dir
	int dirSize = 0;
	foreach (QString filePath, filePathes)
	{
		QFileInfo fileInfo(filePath);
		dirSize += fileInfo.size();
	}
	if (dirSize == 0)
	{
		emit showInfoTip(tr("The dir's file is empty"));
		return;
	}

	if (dirSize > kMaxDirFileSize)
	{
		emit showInfoTip(tr("The dir's file size is no more than %1MB").arg(kMaxDirFileSize/(1024*1024)));
		return;
	}

	QString dirPath = QDir::toNativeSeparators(dir.absolutePath());
	if (!canInsertFile(dirPath))
		return;

	QTextCursor cursor = textCursor();

	QTextCharFormat fileCharFormat = cursor.charFormat();
	fileCharFormat.setObjectType(PM::FileTextFormat);

	QIcon icon = IconProvider::dirIcon();
	QSize s = icon.actualSize(QSize(16,16));
	QImage img = icon.pixmap(s).toImage();

	fileCharFormat.setProperty(PM::FileIcon, img);
	fileCharFormat.setProperty(PM::FileName, dir.dirName());
	fileCharFormat.setProperty(PM::FilePath, dirPath);
	fileCharFormat.setToolTip(dirPath);
	fileCharFormat.setVerticalAlignment(QTextCharFormat::AlignBaseline);

	cursor.insertText(QString(QChar::ObjectReplacementCharacter), fileCharFormat);
	setTextCursor(cursor);
}

void CChatInputBox::insertImage(const QString &rsFile, bool addImageElement /*= true*/)
{
	if (!QFile::exists(rsFile))
	{
		qWarning("CChatInputBox::insertImage: file doesn't exist.");
		return;
	}

	QFile imageFile(rsFile);
	qint64 fileSize = imageFile.size();
	if (fileSize > 5*1024*1024)
	{
		emit showInfoTip(tr("The image can't be larger than 5MB, please use attach to send"));
		return;
	}

	bool fromData = false;
	QImage image = ImageUtil::readImage(rsFile, &fromData);
	if (image.isNull())
	{
		qWarning("CChatInputBox::insertImage: image is null");
		return;
	}

	QString sPath = rsFile;
	if (fromData)
	{
		qWarning("CChatInputBox::insertImage: image need to transfrom");

		// self settings
		QFileInfo fileInfo(rsFile);
		QString suffix = fileInfo.suffix();
		sPath = QString("%1/%2.%3").arg(Account::instance()->imagePath())
			.arg(image.cacheKey())
			.arg(suffix.isEmpty() ? QString("jpg") : suffix);
		bool saveOK = ImageUtil::saveImage(image, sPath);
		if (!saveOK)
		{
			qWarning("CChatInputBox::insertImage: transform image failed");
			return;
		}
	}
	
	insertImage(sPath, image, addImageElement);
}

void CChatInputBox::insertImage(QString &rsFile, const QImage &image, bool addImageElement /*= true*/)
{
	if (image.isNull()) 
	{
		qWarning("CChatInputBox::insertImage: attempt to add an invalid image");
		return;
	}

	QString rsUrl = rsFile;
	if (rsUrl.isEmpty())
	{
		// self settings
		rsUrl = QString("%1/%2.%3").arg(Account::instance()->imagePath())
			.arg(image.cacheKey())
			.arg("jpg");
		bool saveOK = ImageUtil::saveImage(image, rsUrl);
		if (!saveOK)
		{
			qWarning("CChatInputBox::insertImage: save image failed when url is empty");
			return;
		}

		rsFile = rsUrl;
	}

	if (!QFile::exists(rsUrl))
	{
		qWarning("CChatInputBox::insertImage: image file doesn't exist");
		return;
	}

	QFile imageFile(rsUrl);
	qint64 fileSize = imageFile.size();
	if (fileSize > 5*1024*1024)
	{
		emit showInfoTip(tr("The image can't be larger than 5MB, please use attach to send"));
		return;
	}

	// configure out uuid of this image
	QString sUrl = rsUrl;
	QString sUuid = "";
	QTextDocument* doc = this->document();
	QVector<QTextFormat> formats = doc->allFormats();
	foreach (QTextFormat f, formats)
	{
		if (f.isImageFormat() && f.toImageFormat().name() == sUrl && m_imageInfos.contains(sUrl))
		{
			sUuid = m_imageInfos[sUrl].first;
			break;
		}
	}

	QImage displayImage = generateImageResource(image);
	if (sUuid.isEmpty())
	{
		// generate image thumbnail
		if (bean::AttachItem::needGenerateThumbnail(rsUrl, image.size()))
		{
			if (!bean::AttachItem::generateThumbnail(rsUrl))
			{
				qWarning("CChatInputBox::insertImage: generate thumbnail failed: %s", rsUrl.toUtf8().constData());
			}
		}

		// generate image uuid
		sUuid = QUuid::createUuid().toString();
		sUuid = sUuid.mid(1, sUuid.length()-2);
		QPair<QString, QSize> imageInfo = qMakePair(sUuid, image.size());
		m_imageInfos[sUrl] = imageInfo;

		// add image resource
		doc->addResource(QTextDocument::ImageResource, sUrl, displayImage);

		QFileInfo fi(sUrl);
		if (fi.suffix().compare("gif", Qt::CaseInsensitive) == 0)
		{
			QMovie *gifMovie = new QMovie(sUrl);
			m_gifMap.insert(sUrl, gifMovie);
			connect(gifMovie, SIGNAL(frameChanged(int)), SLOT(updateGifFrame(int)));
			gifMovie->setCacheMode(QMovie::CacheNone);
			gifMovie->start();
		}
	}

	// insert image element
	if (addImageElement)
	{
		QTextImageFormat format;
		format.setName(sUrl);
		format.setToolTip(tr("Double click to open image"));
		format.setWidth(displayImage.width());
		format.setHeight(displayImage.height());
		QTextCursor cursor = this->textCursor();
		cursor.insertImage(format);
		setTextCursor(cursor);
	}
}

void CChatInputBox::setTextForeground(const QBrush& brush)
{
	QTextCharFormat charFormat = currentCharFormat();
	charFormat.setForeground(brush);
	setCurrentCharFormat(charFormat);

	QTextBlock block = document()->begin();
	while (block.isValid())
	{
		QTextCursor cursor(block);

		QTextBlock::Iterator itr = block.begin();
		while (itr != block.end())
		{
			QTextFragment frag = itr.fragment();
			QTextCharFormat charFormat = frag.charFormat();
			charFormat.setForeground(brush);
			cursor.setPosition(frag.position());
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, frag.length());
			cursor.setCharFormat(charFormat);

			itr++;
		}

		block = block.next();
	}
}

QBrush CChatInputBox::textForeground() const
{
	QTextBlock block = document()->begin();
	while (block.isValid())
	{
		QTextBlock::Iterator itr = block.begin();
		while (itr != block.end())
		{
			QTextCharFormat charFormat = itr.fragment().charFormat();

			if (charFormat.objectType() != PM::FileTextFormat)
			{
				return charFormat.foreground();
			}

			itr++;
		}

		block = block.next();
	}
	return QBrush();
}

void CChatInputBox::setTextBackgroud(const QBrush& brush)
{
	QTextCharFormat charFormat = currentCharFormat();
	charFormat.setBackground(brush);
	setCurrentCharFormat(charFormat);

	QTextBlock block = document()->begin();
	while (block.isValid())
	{
		QTextCursor cursor(block);

		QTextBlock::Iterator itr = block.begin();
		while (itr != block.end())
		{
			QTextFragment frag = itr.fragment();
			QTextCharFormat charFormat = frag.charFormat();

			charFormat.setBackground(brush);
			cursor.setPosition(frag.position());
			cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, frag.length());
			cursor.setCharFormat(charFormat);

			itr++;
		}

		block = block.next();
	}
}

QBrush CChatInputBox::textBackground() const
{
	QTextBlock block = document()->begin();
	while (block.isValid())
	{
		QTextBlock::Iterator itr = block.begin();
		while (itr != block.end())
		{
			QTextCharFormat charFormat = itr.fragment().charFormat();

			if (charFormat.objectType() != PM::FileTextFormat)
			{
				return charFormat.background();
			}

			itr++;
		}

		block = block.next();
	}

	return QBrush();
}

QString CChatInputBox::msgText() const
{
	QString sRet = "";
	// QString sPrint = "";

	bool bEnter = false;
	QTextBlock block = document()->begin();
	while (block.isValid())
	{
		bEnter = false;
		
		QTextBlock::Iterator itr = block.begin();
		for (; itr != block.end(); ++itr)
		{
			QTextCharFormat charFormat = itr.fragment().charFormat();
			int nLength = itr.fragment().length();
			if (charFormat.isImageFormat())
			{
				QTextImageFormat imageFormat = charFormat.toImageFormat();
				QString sStr;
				QString sUuid;
				QString sName = imageFormat.name();
				if (!sName.isEmpty())
				{
					if (sName.startsWith("face"))
					{
						// face image
						QString faceStr = sName.mid(QString("face").length());
						bool convertOk = false;
						int idx = faceStr.toInt(&convertOk);
						if (convertOk && idx < m_faceFileNames.count())
						{
							QString faceName = m_faceNames[idx];
							sStr = QString("[%1]").arg(faceName);
							sStr = sStr.repeated(nLength);
							sRet += sStr;
						}
						bEnter = false;
					}
					else if (m_imageInfos.contains(sName))
					{
						// external image
						sUuid = m_imageInfos[sName].first;
						sStr = "{" + sUuid + "}";
						sStr = sStr.repeated(nLength);
						sRet += sStr;

						bEnter = true;
					}
				}
			}
			else if (charFormat.objectType() == PM::FileTextFormat)
			{
				// do nothing here...
			}
			else if (charFormat.objectType() == PM::AtTextFormat)
			{
				sRet += ("@" + charFormat.property(PM::AtText).toString());
				bEnter = false;
			}
			else
			{
				sRet += itr.fragment().text();
				bEnter = true;
			}
		}

		if (itr == block.end())
		{
			bEnter = true;
		}

		// sPrint += QString(" block[%1] postion: %2 <br>: %3 ").arg(block.blockNumber()).arg(block.position()).arg(bEnter);

		if (bEnter)
			sRet += "\n";
		block = block.next();
	}

	sRet = sRet.left(sRet.length() - 1); // remove the last '/n'
 	// qWarning() << __FUNCTION__ << sPrint << " blockcount: " << document()->blockCount() << sRet;
	return sRet;
}

QStringList CChatInputBox::msgPieces() const
{
	QStringList pieces;
	QString sRet = "";

	bool bEnter = false;
	QTextBlock block = document()->begin();
	while (block.isValid())
	{
		bEnter = false;

		QTextBlock::Iterator itr = block.begin();
		for (; itr != block.end(); ++itr)
		{
			QTextCharFormat charFormat = itr.fragment().charFormat();
			int nLength = itr.fragment().length();
			if (charFormat.isImageFormat())
			{
				QTextImageFormat imageFormat = charFormat.toImageFormat();
				QString sStr;
				QString sUuid;
				QString sName = imageFormat.name();
				if (!sName.isEmpty())
				{
					if (sName.startsWith("face"))
					{
						// face image
						QString faceStr = sName.mid(QString("face").length());
						bool convertOk = false;
						int idx = faceStr.toInt(&convertOk);
						if (convertOk && idx < m_faceFileNames.count())
						{
							QString faceName = m_faceNames[idx];
							sStr = QString("[%1]").arg(faceName);
							sStr = sStr.repeated(nLength);
							sRet += sStr;
						}
						bEnter = false;
					}
					else if (m_imageInfos.contains(sName))
					{
						if (!sRet.isEmpty())
						{
							pieces.append(sRet);
							sRet.clear();
						}

						// external image
						sUuid = m_imageInfos[sName].first;
						sStr = "{" + sUuid + "}";
						for (int i = 0; i < nLength; ++i)
						{
							pieces.append(sStr);
						}
						bEnter = false;
					}
				}
			}
			else if (charFormat.objectType() == PM::FileTextFormat)
			{
				// do nothing here...
			}
			else if (charFormat.objectType() == PM::AtTextFormat)
			{
				sRet += ("@" + charFormat.property(PM::AtText).toString());
				bEnter = false;
			}
			else
			{
				sRet += itr.fragment().text();
				bEnter = true;
			}
		}

		if (itr == block.end())
		{
			bEnter = true;
		}

		if (bEnter)
			sRet += "\n";

		block = block.next();
	}

	sRet = sRet.left(sRet.length() - 1); // remove the last '/n'
	if (!sRet.isEmpty())
		pieces.append(sRet);

	return pieces;
}

QList<QString> CChatInputBox::atIds() const
{
	QList<QString> lstRet;
	lstRet.clear();

	QTextBlock block = document()->begin();
	while(block.isValid())
	{
		QTextBlock::Iterator itr = block.begin();
		for (; itr != block.end(); ++itr)
		{
			QTextCharFormat format = itr.fragment().charFormat();
			if (format.isImageFormat())
			{
				// do nothing here...
			}
			else if (format.objectType() == PM::FileTextFormat)
			{
				// do nothing here...
			}
			else if (format.objectType() == PM::AtTextFormat)
			{
				// add valid at id
				lstRet.append(format.property(PM::AtId).toString());
			}
			else
			{
				// add invalid at id
				QString bearText = itr.fragment().text();
				int atIndex = bearText.indexOf("@");
				while (atIndex != -1)
				{
					lstRet.append(QString());
					bearText = bearText.mid(atIndex+1);
					atIndex = bearText.indexOf("@");
				}
			}
		}
		block = block.next();
	}

	bool empty = true;
	foreach (QString atId, lstRet)
	{
		if (!atId.isEmpty())
		{
			empty = false;
			break;
		}
	}

	if (empty)
	{
		lstRet.clear();
	}

	return lstRet;
}

QList<QString> CChatInputBox::msgFiles() const
{
	QList<QString> lstRet;
	lstRet.clear();

	QTextBlock block = document()->begin();
	while(block.isValid())
	{
		QTextBlock::Iterator itr = block.begin();
		for (; itr != block.end(); ++itr)
		{
			QTextCharFormat format = itr.fragment().charFormat();
			if (format.objectType() == PM::FileTextFormat)
			{
				lstRet.append(format.property(PM::FilePath).toString());
			}
		}
		block = block.next();
	}

	return lstRet;
}

QMap<QString, QPair<QString, QSize>> CChatInputBox::msgImages() const
{
	QMap<QString, QPair<QString, QSize>> images;
	images.clear();

	QTextBlock block = document()->begin();
	while(block.isValid())
	{
		QTextBlock::Iterator itr = block.begin();
		for (; itr != block.end(); ++itr)
		{
			QTextCharFormat format = itr.fragment().charFormat();
			if (format.isImageFormat())
			{
				QString imageUrl = format.toImageFormat().name();
				if (m_imageInfos.contains(imageUrl))
				{
					images.insert(imageUrl, m_imageInfos[imageUrl]);
				}
			}
		}
		block = block.next();
	}

	return images;
}

void CChatInputBox::clearResource()
{
	qDeleteAll(m_facesMap.values());
	m_facesMap.clear();

	qDeleteAll(m_gifMap.values());
	m_gifMap.clear();

	m_imageInfos.clear();
}

static QString s_copyMessageFormat = "application/message.copy";
QMimeData *CChatInputBox::msgToCopyMimeData(const QString &title, const QString &msgText, 
	                                        const QString &imagePath, const QString &attachPath, const QString &dirPath)
{
	QVariantMap vm;
	vm["title"] = title;
	vm["text"] = msgText;
	vm["image"] = imagePath;
	vm["file"] = attachPath;
	vm["dir"] = dirPath;
	QByteArray data = QtJson::serialize(vm);
	QMimeData *mimeData = new QMimeData();
	mimeData->setData(s_copyMessageFormat, data);
	
	QString mimeText = title;
	if (!mimeText.isEmpty())
		mimeText.append("\n");
	mimeText.append(msgText);
	if (!mimeText.isEmpty())
		mimeData->setText(mimeText);

	if (!imagePath.isEmpty())
		mimeData->setImageData(QImage(imagePath));

	QList<QUrl> urls;
	if (!attachPath.isEmpty())
		urls.append(QUrl::fromLocalFile(attachPath));
	if (!dirPath.isEmpty())
		urls.append(QUrl::fromLocalFile(dirPath));
	if (!urls.isEmpty())
		mimeData->setUrls(urls);

	return mimeData;
}

bool CChatInputBox::copyMimeDataToMsg(const QMimeData *mimeData, QString &title, QString &msgText, 
	                                  QString &imagePath, QString &attachPath, QString &dirPath)
{
	if (!mimeData)
		return false;

	if (!mimeData->hasFormat(s_copyMessageFormat))
		return false;

	QByteArray data = mimeData->data(s_copyMessageFormat);
	QVariantMap vm = QtJson::parse(QString::fromUtf8(data)).toMap();
	title = vm["title"].toString();
	msgText = vm["text"].toString();
	imagePath = vm["image"].toString();
	attachPath = vm["file"].toString();
	dirPath = vm["dir"].toString();
	return true;
}

void CChatInputBox::insertFace(int idx, bool addImageElement /*= true*/)
{
	if (idx < 0 || idx >= m_faceFileNames.count())
		return;

	QString faceFileName = m_faceFileNames[idx];
	if (addImageElement)
	{
		QTextImageFormat imageFormat;
		QString faceName = QString("face%1").arg(idx);
		imageFormat.setName(faceName);
		textCursor().insertImage(imageFormat);
		setTextCursor(textCursor());
	}

	if(!m_facesMap.contains(idx))
	{
		m_facesMap[idx] = new QMovie(faceFileName);
		connect(m_facesMap[idx], SIGNAL(frameChanged(int)), SLOT(updateFaceFrame(int)));
		m_facesMap[idx]->setCacheMode(QMovie::CacheNone);
		m_facesMap[idx]->start();
	}
}

void CChatInputBox::updateFaceFrame(int /*frame*/)
{
	QMovie *movie = qobject_cast<QMovie *>(sender());
	if (movie)
	{
		int idx = m_facesMap.key(movie);
		QString faceName = QString("face%1").arg(idx);
		QUrl url(faceName);
		document()->addResource(QTextDocument::ImageResource, url, movie->currentPixmap());  // use current frame
		setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
	}
}

void CChatInputBox::updateGifFrame(int /*frame*/)
{
	QMovie *movie = qobject_cast<QMovie *>(sender());
	if (movie)
	{
		foreach (QString gifUrl, m_gifMap.keys())
		{
			if (movie == m_gifMap[gifUrl])
			{
				QImage frame = movie->currentImage();
				QImage displayFrame = generateImageResource(frame);
				document()->addResource(QTextDocument::ImageResource, gifUrl, displayFrame);  // use current frame
				setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
			}
		}
	}
}

void CChatInputBox::clear()
{
	clearResource();

	QTextEdit::clear();
}

void CChatInputBox::textCharFormatChanged(const QString &fontFamily, int fontSize, bool bBold, bool bItalic, bool bUnderline, const QColor &color)
{
	setStyleSheet(QString("QTextEdit{"
		"font: %1 %2 %3pt %4;"
		"color: %5;"
		"text-decoration: %6;"
		"}")
		.arg(bBold ? "bold" : "")
		.arg(bItalic ? "italic" : "")
		.arg(fontSize)
		.arg(fontFamily)
		.arg(color.name())
		.arg(bUnderline ? "underline" : "none"));
}

void CChatInputBox::onTextChanged()
{
	if (toPlainText().isEmpty())
	{
		clearResource();
	}
	
	// check if need to do the completion
	performCompletion();

	// make cursor visible
	ensureCursorVisible();
}

void CChatInputBox::addAtText(const QString &atText)
{
	// get name and id
	QString name;
	QString id;
	int leftBraceIndex = atText.lastIndexOf("(");
	int rightBraceIndex = atText.lastIndexOf(")");
	if (leftBraceIndex < rightBraceIndex)
	{
		name = atText.left(leftBraceIndex);
		id = atText.mid(leftBraceIndex+1, rightBraceIndex-leftBraceIndex-1);
	}

	// append at text object
	QTextCursor cursor = textCursor();
	QTextCharFormat atCharFormat = cursor.charFormat();
	atCharFormat.setObjectType(PM::AtTextFormat);
	atCharFormat.setProperty(PM::AtText, name);
	atCharFormat.setProperty(PM::AtId, id);
	atCharFormat.setToolTip(atText);
	atCharFormat.setVerticalAlignment(QTextCharFormat::AlignBaseline);
	cursor.insertText(QString(QChar::ObjectReplacementCharacter), atCharFormat);
	setTextCursor(cursor);

	// append a space
	cursor = textCursor();
	cursor.insertText(" ");
	setTextCursor(cursor);
}

void CChatInputBox::insertMimeData(const QMimeData *source)
{
	if (canInsertFromMimeData(source))
	{
		insertFromMimeData(source);
	}
}

bool CChatInputBox::canInsertFromMimeData(const QMimeData *source) const
{
	if (!acceptRichText())
	{
		if (source->hasText())
			return true;
		else
			return false;
	}

	if (source->hasImage() || 
		source->hasText() || 
		source->hasUrls() || 
		source->hasFormat(s_copyMessageFormat) ||
		QTextEdit::canInsertFromMimeData(source))
		return true;
	else
		return false;
}

void CChatInputBox::insertFromMimeData(const QMimeData *source)
{
	/* // may be slow or exception because of print data operation
	QString sPrint = "{\n";
	foreach(QString sFormat, source->formats())
	{
		sPrint += "[" + sFormat + " : " + QString(source->data(sFormat)) + "]\n";
	}
	sPrint += "}";
	qWarning() << __FUNCTION__ << sPrint;
	*/

	qWarning() << __FUNCTION__ << source->formats();

	if (!canInsertFromMimeData(source))
		return;

	if (!acceptRichText())
	{
		// parse inner link text
		if (source->hasUrls())
		{
			foreach (QUrl url, source->urls())
			{
				if (url.isValid())
				{
					if (url.scheme() == "qrc" && source->hasText()) // inner url link
					{
						QString urlText = url.toString();
						int urlIndex = urlText.indexOf("#");
						if (urlIndex > 0)
						{
							urlText = urlText.mid(urlIndex+1);
							QTextCursor cursor = this->textCursor();
							cursor.beginEditBlock();
							cursor.insertText(urlText);
							cursor.endEditBlock();
							return;
						}
					}
				}
			}
		}

		QString plainText = source->text();
		plainText.remove(QChar(QChar::ObjectReplacementCharacter), Qt::CaseInsensitive);
		insertPlainText(plainText);
		return;
	}

	// copy image
	if (source->hasFormat(s_copyMessageFormat))
	{
		QString title;
		QString msgText;
		QString imagePath;
		QString attachPath;
		QString dirPath;
		copyMimeDataToMsg(source, title, msgText, imagePath, attachPath, dirPath);
		if (!title.isEmpty())
		{
			insertPlainText(title+QString("\n"));
		}
		if (!msgText.isEmpty())
		{
			insertPlainText(msgText);
		}
		if (!imagePath.isEmpty())
		{
			insertImage(imagePath);
		}
		if (!attachPath.isEmpty())
		{
			insertFile(attachPath);
		}
		if (!dirPath.isEmpty())
		{
			insertDir(dirPath);
		}
		return;
	}

	// self edit format
	const QString kSelfFormat = QString("application/vnd.oasis.opendocument.text");
	bool isSelfDocument = source->hasFormat(kSelfFormat);

	if (source->hasImage())
	{
		QImage image = qvariant_cast<QImage>(source->imageData());

		QUrl url;
		if (source->urls().length() > 0)
		{
			url = source->urls().at(0);
		}
		else
		{
			// self settings
			QString sPath = QString("%1/%2.jpg")
				.arg(Account::instance()->imagePath())
				.arg(image.cacheKey());
			image.save(sPath);
			url = QUrl::fromLocalFile(sPath);
		}

		// face drag from web view, url is like this: "qrc:/face/13.gif"
		if (url.scheme() == "qrc" && source->hasText())
		{
			QString faceFileName = url.toString().mid(QString("qrc").length());
			if (m_faceFileNames.contains(faceFileName))
			{
				int idx = m_faceFileNames.indexOf(faceFileName);
				insertFace(idx);
			}
			return;
		}

		QString urlString = url.toString();
		if (url.scheme() == QString::fromLatin1("file"))
		{
			urlString = url.toLocalFile();
		}
		insertImage(urlString, image);
		return;
	}

	if (source->hasUrls())
	{
		if (source->formats().contains(kMessageImageMimeType))
		{
			// from chat browser
			QString urlString = QString::fromUtf8(source->data(kMessageImageMimeType));
			insertImage(urlString, true);
			return;
		}

		foreach (QUrl url, source->urls())
		{
			if (url.isValid())
			{
				QString sFile = url.toLocalFile();
				if (!sFile.isEmpty()) // local file or dir
				{
					QFileInfo fi(sFile);
					if (fi.isDir())
					{
						insertDir(sFile);
					}
					else
					{
						insertFile(sFile);
					}
				}
				else if (url.scheme() == "qrc" && source->hasText()) // inner url link
				{
					QString urlText = url.toString();
					int urlIndex = urlText.indexOf("#");
					if (urlIndex > 0)
					{
						urlText = urlText.mid(urlIndex+1);
						QTextCursor cursor = this->textCursor();
						cursor.beginEditBlock();
						cursor.insertText(urlText);
						cursor.endEditBlock();
					}
					else
					{
						insertBasicMimeData(source);
						return;
					}
				}
				else
				{
					insertBasicMimeData(source);
					return;
				}
			}
		}
		return;
	}

	if (source->hasHtml() && isSelfDocument)
	{
		QString html = source->html();
		insertHtml(html);

		// check if there's face node "<img src="face36" />"
		// if the face node exists, insert the face gif animation
		while (true)
		{
			int imgStartIndex = html.indexOf("<img");
			if (imgStartIndex == -1)
				break;
			int imgStopIndex = html.indexOf(">", imgStartIndex);
			if (imgStopIndex == -1)
				break;

			QString imgElement = html.mid(imgStartIndex, imgStopIndex+1-imgStartIndex);
			html = html.mid(imgStopIndex+1);

			QDomDocument dom;
			if (dom.setContent(imgElement))
			{
				QString src = dom.documentElement().attribute("src");
				if (src.startsWith("face")) // face
				{
					src = src.mid(QString("face").length());
					bool convertOk = false;
					int idx = src.toInt(&convertOk);
					if (convertOk && idx < m_faceFileNames.count())
					{
						if (!isSelfDocument)
							insertFace(idx, true);
						else
							insertFace(idx, false);
					}
				}
				else // local file
				{
					if (!isSelfDocument)
						insertImage(src, true);
					else
						insertImage(src, false);
				}
			}
		}

		return;
	}

	insertBasicMimeData(source);
}

void CChatInputBox::keyPressEvent(QKeyEvent *e)
{
	if (m_completer->popup()->isVisible()) 
	{
		switch (e->key())
		{
		case Qt::Key_Up:
		case Qt::Key_Down:
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
			e->ignore();
			return;
		default:
			m_completer->popup()->hide();
			break;
		}

		QTextEdit::keyPressEvent(e);
		return;
	}

	// self settings
	int nSendType = 0;
	AccountSettings* accountSettings = Account::settings();
	if (accountSettings)
		nSendType = accountSettings->getSendType();

	if (nSendType == 0)
	{
		// send: Enter  return: Ctrl + Enter 
		if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
		{
			if (e->key() == Qt::Key_Return && e->modifiers() == Qt::ControlModifier)
			{
				e->setModifiers(Qt::NoModifier);
				QTextEdit::keyPressEvent(e);
			}
			else if (e->key() == Qt::Key_Enter && e->modifiers() == (Qt::ControlModifier|Qt::KeypadModifier))
			{
				e->setModifiers(Qt::KeypadModifier);
				QTextEdit::keyPressEvent(e);
			}
			else
			{
				emit sendMessage();
			}
		}
		else
			QTextEdit::keyPressEvent(e);
	}
	else
	{
		// send: Ctrl + Enter return: Enter
		if ((e->key() == Qt::Key_Return && e->modifiers() == Qt::ControlModifier) ||
			(e->key() == Qt::Key_Enter && e->modifiers() == (Qt::ControlModifier|Qt::KeypadModifier)))
			emit sendMessage();
		else
			QTextEdit::keyPressEvent(e);
	}
}

void CChatInputBox::focusInEvent(QFocusEvent *e)
{
	ensureCursorVisible();
	QTextEdit::focusInEvent(e);
}

void CChatInputBox::contextMenuEvent(QContextMenuEvent *event)
{
	QTextCursor cursor = cursorForPosition(event->pos());
	QTextCharFormat tcf = cursor.charFormat();
	if (!tcf.isImageFormat())
	{
		return QTextEdit::contextMenuEvent(event);
	}
	QString imgPath = tcf.toImageFormat().name();
	QFileInfo fi(imgPath);
	if (!fi.exists())
	{
		return QTextEdit::contextMenuEvent(event);
	}

	QMenu *menu = createStandardContextMenu();
	menu->clear();
	m_actionCopyImage->setData(imgPath);
	menu->addSeparator();
	menu->addAction(m_actionCopyImage);
	m_actionSaveImage->setData(imgPath);
	menu->addAction(m_actionSaveImage);
	menu->exec(event->globalPos());
	delete menu;
	menu = 0;
}

void CChatInputBox::mouseDoubleClickEvent(QMouseEvent *e)
{
	QTextCursor cursor = cursorForPosition(e->pos());
	QTextCharFormat tcf = cursor.charFormat();
	if (!tcf.isImageFormat())
	{
		return QTextEdit::mouseDoubleClickEvent(e);
	}

	// open images
	QString imgPath = tcf.toImageFormat().name();
	if (!QFile::exists(imgPath))
	{
		return QTextEdit::mouseDoubleClickEvent(e);
	}

	QStringList imgPathes;
	QTextBlock block = document()->begin();
	while(block.isValid())
	{
		QTextBlock::Iterator itr = block.begin();
		for (; itr != block.end(); ++itr)
		{
			tcf = itr.fragment().charFormat();
			if (tcf.isImageFormat())
			{
				if (QFile::exists(tcf.toImageFormat().name()))
					imgPathes << tcf.toImageFormat().name();
			}
		}
		block = block.next();
	}
	qPmApp->openImages(imgPathes, imgPath);
}

bool CChatInputBox::canInsertFile(const QString& rsFile)
{
	QTextBlock block = document()->begin();
	while (block.isValid())
	{
		QTextBlock::Iterator itr = block.begin();
		for (; itr != block.end(); ++itr)
		{
			QTextCharFormat format = itr.fragment().charFormat();
			if (format.objectType() == PM::FileTextFormat &&
				rsFile == format.property(PM::FilePath).toString())
			{
				return false;
			}
		}
		block = block.next();
	}

	return true;
}

void CChatInputBox::initCompleter()
{
	m_completerModel = new QStandardItemModel(this);
	m_completer = new QCompleter(this);
	m_completer->setWidget(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setModel(m_completerModel);
	m_completer->setModelSorting(QCompleter::UnsortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(true);
	m_completer->setCompletionRole(PINYIN_ROLE);

	if (m_completer->popup())
	{
		m_completer->popup()->setAttribute(Qt::WA_Hover, true);
		m_completer->popup()->setMouseTracking(true);
		m_completer->popup()->setItemDelegate(new ComboItemDelegate(m_completer->popup()));
	}

	connect(m_completer, SIGNAL(activated(QModelIndex)), this, SLOT(insertCompletion(QModelIndex)));
}

void CChatInputBox::performCompletion(const QString &completionPrefix)
{
	if (completionPrefix != m_completionPrefix)
	{
		QStringList quanPin = PinyinConveter::instance().quanpin(completionPrefix);
		QString quanPinStr = quanPin.join("");
		m_completer->setCompletionPrefix(quanPinStr);

		m_completionPrefix = completionPrefix;
	}

	if (m_completer->completionCount() >= 1)
	{
		QRect rect = cursorRect();
		rect.setWidth(220 + m_completer->popup()->verticalScrollBar()->sizeHint().width());
		m_completer->complete(rect);
		m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
	}
	else
	{
		m_completionPrefix = "";
		m_completer->setCompletionPrefix("");
		
		if (m_completer->popup()->isVisible())
		{
			m_completer->popup()->hide();
		}
	}
}

void CChatInputBox::performCompletion()
{
	QTextCursor cursor = textCursor();

	// remember current position
	int position = cursor.position();

	// select previous word
	cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor, 1);

	QString completionPrefix = cursor.selectedText();
	if (completionPrefix.right(1).compare("@", Qt::CaseInsensitive) == 0)
	{
		// recover the position
		cursor.setPosition(position, QTextCursor::MoveAnchor);

		// do completion
		performCompletion("");

		return;
	}

	// select last character
	cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 1);
	QString at = cursor.selectedText();
	
	// recover the position
	cursor.setPosition(position, QTextCursor::MoveAnchor);

	// if this word has a "@", do complete
	if (at.startsWith("@", Qt::CaseInsensitive))
	{
		performCompletion(completionPrefix);
	}
}

bool CChatInputBox::caseInsensitiveLessThan(const QString &a, const QString &b)
{
	return a.compare(b, Qt::CaseInsensitive) < 0;
}

QImage CChatInputBox::generateImageResource(const QImage &image)
{
	QImage displayImage = image;
	if (displayImage.height() > kMaxDisplayImageHeight || displayImage.width() > width())
		displayImage = image.scaled(QSize(kMaxDisplayImageWidth, kMaxDisplayImageHeight), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	const int kSpace = 1;
	int height = displayImage.height()+2*kSpace;
	if (height < kMinDisplayImageHeight)
		height = kMinDisplayImageHeight;
	QImage imageResource(displayImage.width()+2*kSpace, height, displayImage.format());
	imageResource.fill(Qt::white);
	QPainter painter(&imageResource);
	painter.drawImage(QPoint(kSpace, imageResource.height()-kSpace-displayImage.height()), displayImage);
	return imageResource;
}

void CChatInputBox::insertBasicMimeData(const QMimeData *source)
{
	if (!source)
		return;

	if (source->hasText())
	{
		QTextCursor cursor = this->textCursor();
		cursor.beginEditBlock();
		cursor.insertText(source->text());
		cursor.endEditBlock();
	}
}

void CChatInputBox::copyImage()
{
	QString fileName = m_actionCopyImage->data().toString();

	QImage img = ImageUtil::readImage(fileName);
	if (img.isNull())
	{
		qWarning("CChatInputBox::copyImage: image is null");
		return;
	}

	QMimeData *mimeData = new QMimeData();
	mimeData->setImageData(img);
	mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(fileName));
	QApplication::clipboard()->setMimeData(mimeData);
	return;
}

void CChatInputBox::saveImage()
{
	QString fileName = m_actionSaveImage->data().toString();
	QImage img = ImageUtil::readImage(fileName);
	if (img.isNull())
	{
		qWarning() << Q_FUNC_INFO << "image is null: " << fileName;
		return;
	}

	QFileInfo fi(fileName);
	QString suffix = fi.suffix();
	QString baseName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	
	QString saveDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
		saveDir = accountSettings->getCurDir();
	
	QString saveName = QString("%1/%2.%3").arg(saveDir).arg(baseName).arg(suffix);
	QString newFileName = FileDialog::getImageSaveFileName(this, tr("Save Image"), saveName);
	if (newFileName.isEmpty())
		return;

	QFileInfo newFi(newFileName);
	if (fi == newFi)
	{
		qWarning() << Q_FUNC_INFO << "save image to same name: " << fileName << newFileName;
		return;
	}

	if (!ImageUtil::saveImage(fileName, newFileName))
	{
		qWarning() << Q_FUNC_INFO << "save image failed: " << fileName << newFileName;
	}

	if (accountSettings)
	{
		accountSettings->setCurDir(newFi.absoluteDir().absolutePath());
	}
}

void CChatInputBox::insertCompletion(const QModelIndex &completionIndex)
{
	if (!completionIndex.isValid())
	{
		return;
	}

	// get completion name and id
	QString name = completionIndex.data(Qt::DisplayRole).toString();
	QString id = completionIndex.data(ATID_ROLE).toString();
	if (name.isEmpty() || id.isEmpty())
		return;

	// remove the @ text
	QTextCursor cursor = textCursor();
	int removedAtTextLen = m_completionPrefix.length() + 1;
	cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, removedAtTextLen);
	cursor.removeSelectedText();
	setTextCursor(cursor);

	m_completionPrefix = "";
	m_completer->setCompletionPrefix("");

	QString completion = QString("%1(%2)").arg(name).arg(id);
	addAtText(completion);
}
