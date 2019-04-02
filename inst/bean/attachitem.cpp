#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QDebug>
#include <QImage>
#include "Account.h"
#include "attachitem.h"
#include "util/FileUtil.h"

namespace bean
{
	class AttachItemData : public QSharedData
	{
	public:
		AttachItemData()
			: size(0)
			, playtime(0)
			, picWidth(0)
			, picHeight(0)
			, ttype(AttachItem::Type_Default)
			, tresult(AttachItem::Transfer_InValid)
		{}

		AttachItemData(const AttachItemData &other)
		{
			uuid     = other.uuid;
			msgType  = other.msgType;
			format   = other.format;
			fname    = other.fname;
			size     = other.size;
			fpath    = other.fpath;
			source   = other.source;
			from     = other.from;
			playtime = other.playtime;
			ttype    = other.ttype;
			tresult  = other.tresult;
			picWidth = other.picWidth;
			picHeight= other.picHeight;
		}

		virtual ~AttachItemData() {}

		MessageType msgType;
		QString uuid;
		QString format;
		QString fname;
		qint64  size;   // file size
		QString fpath;  // file path
		QString source; // source
		QString from;   // uid
		int     picWidth;
		int     picHeight;
		int     playtime; 
		int     ttype;
		int     tresult;
	};

	AttachItem::AttachItem(const QString &filepath /*= ""*/)
		: d(new AttachItemData)
	{
		d->msgType = Message_Invalid;
		init(filepath);
	}

	AttachItem::AttachItem(const AttachItem &other)
		: d(other.d)
	{
	}

	AttachItem::~AttachItem()
	{
	}

	void AttachItem::copy()
	{
		d.detach();
	}

	MessageType AttachItem::messageType() const
	{
		return d->msgType;
	}

	QString AttachItem::from() const
	{
		return d->from;
	}

	QString AttachItem::uuid() const
	{
		return d->uuid;
	}

	QString AttachItem::filename() const
	{
		return d->fname;
	}

	QString AttachItem::format() const
	{
		return d->format;
	}

	qint64 AttachItem::size() const
	{
		return d->size;
	}

	QString AttachItem::filepath() const
	{
		return d->fpath;
	}

	int AttachItem::time() const
	{
		return d->playtime;
	}

	int AttachItem::transferType() const
	{
		return d->ttype;
	}

	int AttachItem::transferResult() const
	{
		return d->tresult;
	}

	QString AttachItem::transfername() const 
	{
		return  QString("%1.%2.%3").arg(d->from).arg(d->uuid).arg(d->format);
	}

	QString AttachItem::savedFname() const
	{
		QString savedFileName;
		if (!d->fpath.isEmpty())
		{
			QFileInfo fi(d->fpath);
			savedFileName = fi.fileName();
		}

		if (savedFileName.isEmpty())
		{
			savedFileName = d->fname;
		}

		return savedFileName;
	}

	QString AttachItem::source() const
	{
		return d->source;
	}

	int AttachItem::picWidth() const
	{
		return d->picWidth;
	}

	int AttachItem::picHeight() const
	{
		return d->picHeight;
	}

	void AttachItem::setMessageType(MessageType type)
	{
		d->msgType = type;
	}

	void AttachItem::setFrom(const QString &from)
	{
		QString bareId = Account::idFromFullId(from);
		d->from = bareId; 
	}

	void AttachItem::setUuid(const QString &uuid)
	{
		if (uuid.isEmpty())
		{
			d->uuid = QUuid::createUuid().toString();
			d->uuid = d->uuid.mid(1, d->uuid.length()-2);
		}
		else
		{
			d->uuid = uuid; 
		}
	}

	void AttachItem::setFilename(const QString &filename)
	{
		d->fname = filename; 
	}

	void AttachItem::setFormat(const QString &format)
	{
		d->format = format.isEmpty() ? "null" : format;
	}

	void AttachItem::setSize(qint64 size)
	{
		d->size = size; 
	}

	void AttachItem::setFilePath(const QString &filepath)
	{
		if (filepath.isEmpty())
			return;

		d->fpath = QDir::toNativeSeparators(filepath);
	}

	void AttachItem::setTime(int time)
	{
		d->playtime = time;
	}

	void AttachItem::setTransferType(int type)
	{
		d->ttype = type;
	}

	void AttachItem::setTransferResult(int result)
	{
		d->tresult = result;
	}

	void AttachItem::setSource(const QString &source)
	{
		d->source = source;
	}

	void AttachItem::setPicWidth(int width)
	{
		d->picWidth = width;
	}

	void AttachItem::setPicHeight(int height)
	{
		d->picHeight = height;
	}

	QVariantMap AttachItem::toVariantMap()
	{
		QVariantMap ret;
		ret[bean::kszName]      = d->fname;
		ret[bean::kszSavedName] = this->savedFname();
		ret[bean::kszFormat]    = d->format;
		ret[bean::kszSize]      = d->size;
		ret[bean::kszPath]      = QString(QUrl::fromLocalFile(d->fpath).toEncoded());
		ret[bean::kszUuid]      = d->uuid;
		ret[bean::kszTime]      = d->playtime;
		QString sType = "";
		switch (d->ttype)
		{
		case Type_AutoDisplay:
			sType = bean::kszAutoDisplay;
			break;
		case Type_AutoDownload:
			sType = bean::kszAutoDownload;
			break;
		case Type_Dir:
			sType = bean::kszDir;
			break;
		}
		ret[bean::kszType] = sType;

		QString sResult = "";
		switch (d->tresult)
		{
		case Transfer_InValid:
			sResult = "";
			break;
		case Transfer_Successful:
			sResult = bean::kszSuccessful;
			break;
		case Transfer_Cancel:
			sResult = bean::kszCancel;
			break;
		case Transfer_Error:
			sResult = bean::kszError;
			break;
		default:
			break;
		}

		ret[bean::kszResult] = sResult;
		ret[bean::kszSource] = d->source;
		ret[bean::kszPicWidth] = d->picWidth;
		ret[bean::kszPicHeight] = d->picHeight;

		return ret;
	}

	QVariantMap AttachItem::toAttachMap(quint64 msgId, bool containResult /*= false*/) const
	{   
		//      1         2      3        4         5      6       7         8        9     10       11         12
		// [messageid] [name] [format] [filename] [uuid] [size] [fttype] [ftresult] [time] [source] [picwidth]  [picheight]
		QVariantMap vmap;
		vmap.insert("messageid", msgId);
		vmap.insert("name", this->filename());
		vmap.insert("format", this->format());
		vmap.insert("filename", this->filepath());
		vmap.insert("uuid", this->uuid());
		vmap.insert("size", this->size());
		vmap.insert("fttype", int(this->transferType()));
		if (containResult)
			vmap.insert("ftresult", int(this->transferResult()));
		vmap.insert("time", this->time());
		vmap.insert("source", this->source());
		vmap.insert("picwidth", this->picWidth());
		vmap.insert("picheight", this->picHeight());
		return vmap;
	}

	void AttachItem::fromAttachList(const QString &from, const QVariantList &vl)
	{
		//      1         2      3        4         5      6       7         8        9     10       11         12
		// [messageid] [name] [format] [filename] [uuid] [size] [fttype] [ftresult] [time] [source]	[picwidth]  [picheight]
		this->setFrom(from);
		this->setFilename(vl[2].toString());
		this->setFormat(vl[3].toString());
		this->setFilePath(vl[4].toString());
		this->setUuid(vl[5].toString());
		this->setSize(vl[6].toLongLong());
		this->setTransferType(bean::AttachItem::TransferType(vl[7].toInt()));
		this->setTransferResult(bean::AttachItem::TransferResult(vl[8].toInt()));
		this->setTime(vl[9].toInt());
		this->setSource(vl[10].toString());
		this->setPicWidth(vl[11].toInt());
		this->setPicHeight(vl[12].toInt());
	}

	bool AttachItem::isValid()
	{
		bool bRet = false;
		do 
		{
			if (d->uuid.isEmpty())
				break;

			if (d->format.isEmpty())
				break;

			if (d->fname.isEmpty())
				break;

			if (d->size <= 0)
				break;
			bRet = true;
		} while (0);

		return bRet;
	}

	QSize AttachItem::getAutoDisplaySize(const QSize &actSize)
	{
		QSize displaySize(actSize);
		if (displaySize.width() >= bean::MAX_IMAGE_DISP_WIDTH || displaySize.height() >= bean::MAX_IMAGE_DISP_HEIGHT)
		{
			displaySize.scale(bean::MAX_IMAGE_DISP_WIDTH, bean::MAX_IMAGE_DISP_HEIGHT, Qt::KeepAspectRatio);
		}
		return displaySize;
	}

	bool AttachItem::needGenerateThumbnail(const QString &filePath, const QSize &actSize)
	{
		QFileInfo fileInfo(filePath);
		if (fileInfo.suffix().toLower() == QString::fromLatin1("gif"))
			return false;

		if (actSize.width() >= bean::MAX_IMAGE_DISP_WIDTH || actSize.height() >= bean::MAX_IMAGE_DISP_HEIGHT)
			return true;
		
		return false;
	}

	QString AttachItem::getAutoDisplayThumbnailPath(const QString &filePath)
	{
		QFileInfo fileInfo(filePath);
		QDir dir = fileInfo.dir();
		QString baseName = fileInfo.completeBaseName();
		QString fileName = baseName + QString("_thumb.jpg");
		QString thumbPath = Account::instance()->thumbPath() + QString("\\") + baseName + QString("_thumb.jpg");
		return QDir::toNativeSeparators(thumbPath);
	}

	bool AttachItem::generateThumbnail(const QString &filePath)
	{
		QImage img;
		if (!img.load(filePath))
			return false;

		QSize sz = getAutoDisplaySize(img.size());
		img = img.scaled(sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QString thumbPath = getAutoDisplayThumbnailPath(filePath);
		QFileInfo fi(thumbPath);
		return img.save(thumbPath, fi.suffix().toLatin1().constData());
	}

	void AttachItem::init(const QString &filepath)
	{
		if (filepath.isEmpty())
			return;

		QFileInfo info(filepath);

		if (d->uuid.isEmpty())
		{
			d->uuid = QUuid::createUuid().toString();
			d->uuid = d->uuid.mid(1, d->uuid.length()-2);
		}

		if (info.isDir())
		{
			// dir
			QDir dir(filepath);
			d->fpath = QDir::toNativeSeparators(dir.absolutePath());
			setFilename(dir.dirName());
			setFormat(QString::fromLatin1(bean::kszDefaultDirExt));
			setSize(FileUtil::dirSize(dir));
			setTransferType(bean::AttachItem::Type_Dir);
		}
		else
		{
			// file
			d->fpath = QDir::toNativeSeparators(info.absoluteFilePath());
			setFilename(info.fileName());
			setFormat(info.suffix());
			setSize(FileUtil::fileSize(filepath));
			setTransferType(bean::AttachItem::Type_Default);
		}
	}

	AttachItem & AttachItem::operator=(const AttachItem &other)
	{
		d = other.d;
		return (*this);
	}

}
