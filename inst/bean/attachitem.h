#ifndef ATTACHITEM_H
#define ATTACHITEM_H

#include <QExplicitlySharedDataPointer>
#include <QString>
#include <QVariantMap>
#include <QSize>
#include "bean.h"

namespace bean
{
	class AttachItemData;
	class AttachItem
	{
	public:
		enum TransferType
		{
			Type_Default = 0,
			Type_AutoDownload,
			Type_AutoDisplay,
			Type_Dir
		};

		enum TransferResult
		{
			Transfer_InValid = 0,
			Transfer_Successful,
			Transfer_Cancel,
			Transfer_Error
		};

	public:
		explicit AttachItem(const QString &filepath = "");
		AttachItem(const AttachItem &other);

		virtual ~AttachItem();

		AttachItem & operator=(const AttachItem &other);

		void copy();

		MessageType messageType() const;
		QString from() const;
		QString uuid() const;
		QString filename() const;
		QString format() const;
		qint64  size() const;
		QString filepath() const;
		int time() const;
		int transferType() const;
		int transferResult() const;
		QString transfername() const;
		QString savedFname() const;
		QString source() const;
		int picWidth() const;
		int picHeight() const;

		void setMessageType(MessageType type);
		void setFrom(const QString &from);
		void setUuid(const QString &uuid);
		void setFilename(const QString &filename);
		void setFormat(const QString &format);
		void setSize(qint64 size);
		void setFilePath(const QString &filepath);
		void setTime(int time);
		void setTransferType(int type);
		void setTransferResult(int result);
		void setSource(const QString &source);
		void setPicWidth(int width);
		void setPicHeight(int height);

		QVariantMap toVariantMap();

		QVariantMap toAttachMap(quint64 msgId, bool containResult = false) const;
		void fromAttachList(const QString &from, const QVariantList &vl);

		bool isValid();

		static QSize getAutoDisplaySize(const QSize &actSize);
		static QString getAutoDisplayThumbnailPath(const QString &filePath);
		static bool needGenerateThumbnail(const QString &filePath, const QSize &actSize);
		static bool generateThumbnail(const QString &filePath);

	private:
		void init(const QString &filepath);

	private:
		QExplicitlySharedDataPointer<AttachItemData> d;
	};
}

Q_DECLARE_METATYPE(bean::AttachItem);


#endif // ATTACHITEM_H
