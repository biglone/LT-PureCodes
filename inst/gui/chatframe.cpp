#include "chatframe.h"
#include <QTextFrame>
#include <QTextTable>
#include <QUrl>
#include "message4js.h"
#include "bean/bean.h"
#include "chatinputdef.h"
#include <QTextDocument>
#include "util/MaskUtil.h"
#include <QPixmap>
#include <QBitmap>

const char *kszCommandSep              = ":";
const char *kszUserIdCommand           = "user_id";
const char *kszAvatarCommand           = "avatar_click";
const char *kszOpenImageCommand        = "open_image";
const char *kszPlayCommand             = "audio_play";
const char *kszStopCommand             = "audio_stop";
const char *kszDownloadCommand         = "download";
const char *kszCancelDownloadCommand   = "cancel_download";
const char *kszOpenFileCommand         = "open_file";
const char *kszOpenDirCommand          = "open_dir";
const char *kszSaveAsCommand           = "save_as";

const QString kUuidReg = QString("\\{[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}\\}");
const int kUuidLength = 38;

const QString kBareUuidReg = QString("[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}");
const int kBareUuidLength = 36;

ChatFrame::ChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent /*= 0*/)
: QObject(parent), 
  m_message4Js(message4Js), 
  m_chatFrame(frame),
  m_avatarBubbleTable(0),
  m_bubbleTable(0),
  m_showTime(true),
  m_frameMode(ModeBubble),
  m_send(true),
  m_group(false),
  m_sync(false)
{
	Q_ASSERT(m_chatFrame);
}

ChatFrame::~ChatFrame()
{

}

void ChatFrame::setShowTime(bool showTime)
{
	m_showTime = showTime;
}

bool ChatFrame::isShowTime() const
{
	return m_showTime;
}

void ChatFrame::setFrameMode(FrameMode mode)
{
	m_frameMode = mode;
}

ChatFrame::FrameMode ChatFrame::frameMode() const
{
	return m_frameMode;
}

void ChatFrame::setData(const QVariantMap &data)
{
	m_data = data;
	m_send = m_data[bean::kszSend].toBool();
	m_group = !m_data[bean::kszGid].toString().isEmpty();
	m_sync = (m_data[bean::kszSync].toInt() == 1);
}

bool ChatFrame::isSend() const
{
	return m_send;
}

bool ChatFrame::isGroup() const
{
	return m_group;
}

bool ChatFrame::isSync() const
{
	return m_sync;
}

bool ChatFrame::isSendAttach() const
{
	if (isSend() && !isSync())
		return true;
	else
		return false;
}

void ChatFrame::makeChatFrame()
{
	if (m_frameMode == ModeBubble)
	{
		if (m_showTime)
			makeBubbleTime();
		makeAvatarBubbleContainer();
		makeBubbleAvatar();
		makeBubbleFrom();
		makeBubble();
		makeContent();
	}
	else
	{
		makeTextTimeAndFrom();
		makeContent();
	}
}

CMessage4Js *ChatFrame::message4Js() const
{
	return m_message4Js;
}

QTextFrame *ChatFrame::frame() const
{
	return m_chatFrame;
}

QTextTable *ChatFrame::avatarBubbleTable() const
{
	return m_avatarBubbleTable;
}

QTextTable *ChatFrame::bubbleTable() const
{
	return m_bubbleTable;
}

QTextTableCell ChatFrame::bubbleContentCell() const
{
	if (isSend())
	{
		return m_bubbleTable->cellAt(1, 1);
	}
	else
	{
		return m_bubbleTable->cellAt(1, 2);
	}
}

QTextCursor ChatFrame::firstContentTextCursor() const
{
	if (frameMode() == ModeBubble)
	{
		QTextTableCell contentCell = bubbleContentCell();
		return contentCell.firstCursorPosition();
	}
	else
	{
		return m_chatFrame->firstCursorPosition();
	}
}

QTextCursor ChatFrame::lastContentTextCursor() const
{
	if (frameMode() == ModeBubble)
	{
		QTextTableCell contentCell = bubbleContentCell();
		return contentCell.lastCursorPosition();
	}
	else
	{
		return m_chatFrame->lastCursorPosition();
	}
}

void ChatFrame::makeAvatarBubbleContainer()
{
	QTextCursor textCursor = m_chatFrame->lastCursorPosition();
	if (isSend())
	{
		QTextTableFormat tableFormat;
		tableFormat.setBorderStyle(QTextTableFormat::BorderStyle_Solid);
		tableFormat.setBorder(1);
		tableFormat.setBorderBrush(Qt::white);
		tableFormat.setCellSpacing(0);
		tableFormat.setCellPadding(0);
		tableFormat.setPosition(QTextTableFormat::FloatRight);
		QTextLength tableLength(QTextLength::PercentageLength, 90);
		tableFormat.setWidth(tableLength);
		QVector<QTextLength> columnWidths;
		columnWidths << QTextLength(QTextLength::VariableLength, 0) << QTextLength(QTextLength::FixedLength, 34);
		tableFormat.setColumnWidthConstraints(columnWidths);
		m_avatarBubbleTable = textCursor.insertTable(1, 2, tableFormat);
	}
	else
	{
		QTextTableFormat tableFormat;
		tableFormat.setBorderStyle(QTextTableFormat::BorderStyle_Solid);
		tableFormat.setBorder(1);
		tableFormat.setBorderBrush(Qt::white);
		tableFormat.setCellSpacing(0);
		tableFormat.setCellPadding(0);
		tableFormat.setPosition(QTextTableFormat::FloatLeft);
		QTextLength tableLength(QTextLength::PercentageLength, 90);
		tableFormat.setWidth(tableLength);
		QVector<QTextLength> columnWidths;
		columnWidths << QTextLength(QTextLength::FixedLength, 34) << QTextLength(QTextLength::VariableLength, 0);
		tableFormat.setColumnWidthConstraints(columnWidths);
		m_avatarBubbleTable = textCursor.insertTable(1, 2, tableFormat);
	}
}

void ChatFrame::makeBubbleAvatar()
{
	QTextTableCell avatarCell;
	QString uid;
	if (isSend())
	{
		avatarCell = m_avatarBubbleTable->cellAt(0, 1);
		QTextTableCellFormat cellFormat = avatarCell.format().toTableCellFormat();
		cellFormat.setRightPadding(3);
		avatarCell.setFormat(cellFormat);
		uid = m_message4Js->getUid();
	}
	else
	{
		avatarCell = m_avatarBubbleTable->cellAt(0, 0);
		QTextTableCellFormat cellFormat = avatarCell.format().toTableCellFormat();
		cellFormat.setLeftPadding(3);
		avatarCell.setFormat(cellFormat);
		uid = m_data[bean::kszUid].toString();
	}

	QString avatarName = m_message4Js->userAvatar(uid);
	QPixmap pixmapAvatar(avatarName);
	const QSize kImageSize(30, 30);
	pixmapAvatar = pixmapAvatar.scaled(kImageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.top = border.bottom = border.left = border.right = 4;
	QBitmap mask = MaskUtil::generateMask(rawMask, border, kImageSize);
	pixmapAvatar.setMask(mask);
	QImage avatar = pixmapAvatar.toImage();
	m_chatFrame->document()->addResource(QTextDocument::ImageResource, avatarName, avatar);

	QTextCursor textCursor = avatarCell.firstCursorPosition();
	QTextImageFormat imageFormat;
	imageFormat.setName(avatarName);
	imageFormat.setAnchor(true);
	imageFormat.setAnchorHref(QString("%1%2%3").arg(kszAvatarCommand).arg(kszCommandSep).arg(uid));
	textCursor.insertImage(imageFormat);
}

void ChatFrame::makeBubble()
{
	// make bubble table
	QTextTableCell bubbleCell;
	QTextTableFormat tableFormat;
	tableFormat.setBorderStyle(QTextTableFormat::BorderStyle_None);
	tableFormat.setBorder(0);
	tableFormat.setCellSpacing(0);
	tableFormat.setCellPadding(0);
	QVector<QTextLength> columnWidths;
	if (isSend())
	{
		tableFormat.setPosition(QTextTableFormat::FloatRight);

		columnWidths << QTextLength(QTextLength::FixedLength, 3);
		columnWidths << QTextLength(QTextLength::VariableLength, 0);
		columnWidths << QTextLength(QTextLength::FixedLength, 3);
		columnWidths << QTextLength(QTextLength::FixedLength, 7);

		bubbleCell = m_avatarBubbleTable->cellAt(0, 0);
	}
	else
	{
		tableFormat.setPosition(QTextTableFormat::FloatLeft);

		columnWidths << QTextLength(QTextLength::FixedLength, 7);
		columnWidths << QTextLength(QTextLength::FixedLength, 3);
		columnWidths << QTextLength(QTextLength::VariableLength, 0);
		columnWidths << QTextLength(QTextLength::FixedLength, 3);

		bubbleCell = m_avatarBubbleTable->cellAt(0, 1);
	}
	tableFormat.setColumnWidthConstraints(columnWidths);
	QTextCursor textCursor = bubbleCell.lastCursorPosition();
	m_bubbleTable = textCursor.insertTable(3, 4, tableFormat);
	for (int i = 0; i < m_bubbleTable->rows(); ++i)
	{
		for (int j = 0; j < m_bubbleTable->columns(); ++j)
		{
			QTextTableCell cell = m_bubbleTable->cellAt(i, j);
			QTextTableCellFormat cellFormat;
			cellFormat.setPadding(0);
			cellFormat.setBackground(Qt::white);
			cell.setFormat(cellFormat);

			if (i == 0 || i == 2)
			{
				QTextBlockFormat blockFormat;
				blockFormat.setLineHeight(3, QTextBlockFormat::FixedHeight);
				QTextCursor textCursor = cell.firstCursorPosition();
				textCursor.mergeBlockFormat(blockFormat);
			}
		}
	}

	/*
	// make bubble image
	QTextTableCell tableCell;
	int bubbleStartColumn = 1;
	QTextImageFormat imageFormat;
	QTextTableCellFormat cellFormat;
	cellFormat.setBackground(QColor(241, 241, 246));
	QString topLeft = QString(":/html/images/recvbubble/2.png");
	QString bottomLeft = QString(":/html/images/recvbubble/4.png");
	QString topRight = QString(":/html/images/recvbubble/8.png");
	QString bottomRight = QString(":/html/images/recvbubble/10.png");
	if (isSend())
	{
		cellFormat.setBackground(QColor(194, 228, 255));
		bubbleStartColumn = 0;

		topLeft = QString(":/html/images/sendbubble/8.png");
		bottomLeft = QString(":/html/images/sendbubble/10.png");
		topRight = QString(":/html/images/sendbubble/2.png");
		bottomRight = QString(":/html/images/sendbubble/4.png");
	}

	tableCell = m_bubbleTable->cellAt(0, bubbleStartColumn);
	textCursor = tableCell.firstCursorPosition();
	imageFormat.setName(topLeft);
	textCursor.insertImage(imageFormat);

	tableCell = m_bubbleTable->cellAt(1, bubbleStartColumn);
	tableCell.setFormat(cellFormat);

	tableCell = m_bubbleTable->cellAt(2, bubbleStartColumn);
	textCursor = tableCell.firstCursorPosition();
	imageFormat.setName(bottomLeft);
	textCursor.insertImage(imageFormat);

	tableCell = m_bubbleTable->cellAt(0, bubbleStartColumn+1);
	tableCell.setFormat(cellFormat);

	tableCell = m_bubbleTable->cellAt(1, bubbleStartColumn+1);
	tableCell.setFormat(cellFormat);

	tableCell = m_bubbleTable->cellAt(2, bubbleStartColumn+1);
	tableCell.setFormat(cellFormat);

	tableCell = m_bubbleTable->cellAt(0, bubbleStartColumn+2);
	textCursor = tableCell.firstCursorPosition();
	imageFormat.setName(topRight);
	textCursor.insertImage(imageFormat);

	tableCell = m_bubbleTable->cellAt(1, bubbleStartColumn+2);
	tableCell.setFormat(cellFormat);

	tableCell = m_bubbleTable->cellAt(2, bubbleStartColumn+2);
	textCursor = tableCell.firstCursorPosition();
	imageFormat.setName(bottomRight);
	textCursor.insertImage(imageFormat);

	// add bubble arrow
	if (!isSend())
	{
		tableCell = m_bubbleTable->cellAt(1, 0);
		cellFormat = tableCell.format().toTableCellFormat();
		cellFormat.setTopPadding(6);
		tableCell.setFormat(cellFormat);

		textCursor = tableCell.firstCursorPosition();
		QString imageString = QString(":/html/images/recvbubble/1.png");
		imageFormat.setName(imageString);
		textCursor.insertImage(imageFormat);
	}
	else
	{
		tableCell = m_bubbleTable->cellAt(1, 3);
		cellFormat = tableCell.format().toTableCellFormat();
		cellFormat.setTopPadding(6);
		tableCell.setFormat(cellFormat);

		textCursor = tableCell.firstCursorPosition();
		QString imageString = QString(":/html/images/sendbubble/1.png");
		imageFormat.setName(imageString);
		textCursor.insertImage(imageFormat);
	}

	// content cell set paddings
	if (!isSend())
	{
		tableCell = m_bubbleTable->cellAt(1, 2);
	}
	else
	{
		tableCell = m_bubbleTable->cellAt(1, 1);
	}
	cellFormat = tableCell.format().toTableCellFormat();
	cellFormat.setLeftPadding(6);
	cellFormat.setRightPadding(6);
	cellFormat.setTopPadding(4);
	cellFormat.setBottomPadding(4);
	tableCell.setFormat(cellFormat);
	*/
}

void ChatFrame::makeBubbleTime()
{
	QString dateTime = m_data[bean::kszTime].toString();
	dateTime = m_message4Js->displayTime(dateTime);

	QTextCursor textCursor = m_chatFrame->lastCursorPosition();
	QTextBlockFormat blockFormat = textCursor.blockFormat();
	blockFormat.setAlignment(Qt::AlignHCenter);
	blockFormat.setBottomMargin(9);
	textCursor.setBlockFormat(blockFormat);
	QTextCharFormat charFormat = textCursor.charFormat();
	charFormat.setForeground(QColor("#999999"));
	charFormat.setFontPointSize(9);
	textCursor.insertText(dateTime, charFormat);
}

void ChatFrame::makeBubbleFrom()
{
	if (!isSend() && isGroup())
	{
		QTextTableCell cell = m_avatarBubbleTable->cellAt(0, 1);
		QTextCursor textCursor = cell.firstCursorPosition();
		QTextBlockFormat blockFormat = textCursor.blockFormat();
		blockFormat.setTextIndent(7);
		textCursor.setBlockFormat(blockFormat);

		QString uid = m_data[bean::kszUid].toString();
		QString uname = m_data[bean::kszUname].toString();
		QTextCharFormat charFormat;
		charFormat.setForeground(QColor("#888"));
		charFormat.setFontPointSize(9);
		textCursor.insertText(uname, charFormat);

		textCursor.insertText(" (", charFormat);

		QTextCharFormat uidCharFormat = charFormat;
		QString uidCmd = QString("%1%2%3").arg(kszUserIdCommand).arg(kszCommandSep).arg(uid);
		uidCharFormat.setAnchor(true);
		uidCharFormat.setAnchorNames(QStringList() << uid);
		uidCharFormat.setAnchorHref(uidCmd);
		uidCharFormat.setFontUnderline(true);
		textCursor.insertText(uid, uidCharFormat);

		textCursor.insertText(")", charFormat);
	}
}

void ChatFrame::makeTextTimeAndFrom()
{

}

QString ChatFrame::localPathFromRawPath(const QString &rawPath)
{
	QUrl urlPath = QUrl::fromEncoded(rawPath.toUtf8());
	QString path = urlPath.toLocalFile();
	return path;
}

void ChatFrame::setAttachResult(const QString &attachUuid, bool ok)
{
	QVariantMap attachs = m_data[bean::kszAttachs].toMap();
	if (attachs.contains(attachUuid))
	{
		QVariantMap attach = attachs[attachUuid].toMap();
		if (ok)
			attach[bean::kszResult] = bean::kszSuccessful;
		else
			attach[bean::kszResult] = bean::kszError;
		attachs[attachUuid] = attach;
		m_data[bean::kszAttachs] = attachs;
	}
}

QVariantMap ChatFrame::attach(const QString &attachUuid) const
{
	QVariantMap attachs = m_data[bean::kszAttachs].toMap();
	QVariantMap attach;
	if (attachs.contains(attachUuid))
		attach = attachs[attachUuid].toMap();
	return attach;
}

bool ChatFrame::hasAttach(const QString &attachUuid) const
{
	QVariantMap attachs = m_data[bean::kszAttachs].toMap();
	return attachs.contains(attachUuid);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS TextChatFrame
TextChatFrame::TextChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent /*= 0*/)
: ChatFrame(frame, message4Js, parent)
{

}

TextChatFrame::~TextChatFrame()
{

}

void TextChatFrame::setData(const QVariantMap &data)
{
	ChatFrame::setData(data);

	QVariantMap attachs = data[bean::kszAttachs].toMap();
	foreach (QString uuid, attachs.keys())
	{
		QVariantMap attach = attachs[uuid].toMap();
		QString attachType = attach[bean::kszType].toString();
		if (attachType == bean::kszAutoDisplay)
		{
			QString rawPath = attach[bean::kszPath].toString();
			QImage image(ChatFrame::localPathFromRawPath(rawPath));
			if (!image.isNull())
				m_imageSizes.insert(uuid, image.size());
			else
				m_imageSizes.insert(uuid, QSize());
		}
	}
}

QSize TextChatFrame::imageActualSize(const QString &attachUuid) const
{
	QSize sz;
	if (!m_imageSizes.contains(attachUuid))
		return sz;

	sz = m_imageSizes[attachUuid];
	return sz;
}

void TextChatFrame::setImageUploadOK(const QString &attachUuid)
{
	if (!m_imageSizes.contains(attachUuid))
		return;

	setAttachResult(attachUuid, true);
}

void TextChatFrame::setImageUploadError(const QString &attachUuid)
{
	if (!m_imageSizes.contains(attachUuid))
		return;

	setAttachResult(attachUuid, false);
}

void TextChatFrame::setImageDownloadOK(const QString &attachUuid, const QString &path, 
									   int actWidth, int actHeight, int dispWidth, int dispHeight)
{
	if (!m_imageSizes.contains(attachUuid))
		return;

	// update result & path
	QVariantMap attachs = m_data[bean::kszAttachs].toMap();
	QVariantMap attach = attachs[attachUuid].toMap();
	attach[bean::kszPath] = path;
	attach[bean::kszResult] = bean::kszSuccessful;
	attachs[attachUuid] = attach;
	m_data[bean::kszAttachs] = attachs;

	// update size
	m_imageSizes[attachUuid] = QSize(actWidth, actHeight);

	// show image
	QTextCursor textCursor = firstContentTextCursor();
	QTextBlock firstBlock = textCursor.block();
	textCursor = lastContentTextCursor();
	QTextBlock lastBlock = textCursor.block();
	QTextBlock block = firstBlock;
	while (block.isValid())
	{
		bool findImage = false;
		QTextBlock::Iterator it = block.begin();
		while (it != block.end())
		{
			QTextFragment fragment = it.fragment();
			QTextCharFormat chatFormat = fragment.charFormat();
			if (chatFormat.isImageFormat())
			{
				QTextImageFormat imageFormat = chatFormat.toImageFormat();
				QStringList anchorNames = imageFormat.anchorNames();
				if (anchorNames.contains(attachUuid))
				{
					QString localPath = localPathFromRawPath(path);
					QImage image(localPath);
					m_chatFrame->document()->addResource(QTextDocument::ImageResource, path, image);

					imageFormat.setName(path);
					imageFormat.setWidth(dispWidth);
					imageFormat.setHeight(dispHeight);
					imageFormat.setAnchor(true);
					imageFormat.setAnchorNames(QStringList() << attachUuid);
					imageFormat.setAnchorHref(QString("%1%2%3").arg(kszOpenImageCommand).arg(kszCommandSep).arg(path));
					textCursor.setPosition(fragment.position());
					textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					textCursor.setCharFormat(imageFormat);
					textCursor.clearSelection();

					findImage = true;
					break;
				}
			}

			++it;
		}

		if (findImage) // already found, quit
			break;

		if (block == lastBlock) // travel to the last block, quit
			break;

		block = block.next();
	}
}

void TextChatFrame::setImageDownloadError(const QString &attachUuid)
{
	if (!m_imageSizes.contains(attachUuid))
		return;

	// update result
	setAttachResult(attachUuid, false);

	// show image
	QTextCursor textCursor = firstContentTextCursor();
	QTextBlock firstBlock = textCursor.block();
	textCursor = lastContentTextCursor();
	QTextBlock lastBlock = textCursor.block();
	QTextBlock block = firstBlock;
	while (block.isValid())
	{
		bool findImage = false;
		QTextBlock::Iterator it = block.begin();
		while (it != block.end())
		{
			QTextFragment fragment = it.fragment();
			QTextCharFormat chatFormat = fragment.charFormat();
			if (chatFormat.isImageFormat())
			{
				QTextImageFormat imageFormat = chatFormat.toImageFormat();
				QStringList anchorNames = imageFormat.anchorNames();
				if (anchorNames.contains(attachUuid))
				{
					QString path = QString(":/images/errorBmp.png");
					imageFormat.setName(path);
					textCursor.setPosition(fragment.position());
					textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					textCursor.setCharFormat(imageFormat);
					textCursor.clearSelection();

					findImage = true;
					break;
				}
			}

			++it;
		}

		if (findImage) // already found, quit
			break;

		if (block == lastBlock) // travel to the last block, quit
			break;

		block = block.next();
	}
}

void TextChatFrame::setImageDisplaySize(const QString &attachUuid, const QSize &dispSize)
{
	if (!m_imageSizes.contains(attachUuid))
		return;

	// show image
	QTextCursor textCursor = firstContentTextCursor();
	QTextBlock firstBlock = textCursor.block();
	textCursor = lastContentTextCursor();
	QTextBlock lastBlock = textCursor.block();
	QTextBlock block = firstBlock;
	while (block.isValid())
	{
		bool findImage = false;
		QTextBlock::Iterator it = block.begin();
		while (it != block.end())
		{
			QTextFragment fragment = it.fragment();
			QTextCharFormat chatFormat = fragment.charFormat();
			if (chatFormat.isImageFormat())
			{
				QTextImageFormat imageFormat = chatFormat.toImageFormat();
				QStringList anchorNames = imageFormat.anchorNames();
				if (anchorNames.contains(attachUuid))
				{
					imageFormat.setWidth(dispSize.width());
					imageFormat.setHeight(dispSize.height());
					textCursor.setPosition(fragment.position());
					textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					textCursor.setCharFormat(imageFormat);
					textCursor.clearSelection();

					findImage = true;
					break;
				}
			}

			++it;
		}

		if (findImage) // already found, quit
			break;

		if (block == lastBlock) // travel to the last block, quit
			break;

		block = block.next();
	}
}

void TextChatFrame::makeContent()
{
	// fix char format
	QString fontFamily = m_data[bean::kszFontFamily].toString();
	int fontSize = m_data[bean::kszFontSize].toInt();
	QString colorName = m_data[bean::kszColor].toString();
	bool bold = m_data[bean::kszBold].toBool();
	bool italic = m_data[bean::kszItalic].toBool();
	bool underline = m_data[bean::kszUnderline].toBool();
	QTextCharFormat charFormat;
	charFormat.setFontFamily(fontFamily);
	charFormat.setFontPointSize(fontSize);
	charFormat.setForeground(QColor(colorName));
	if (bold)
		charFormat.setFontWeight(QFont::Bold);
	charFormat.setFontItalic(italic);
	charFormat.setFontUnderline(underline);

	QVariantMap attachs = m_data[bean::kszAttachs].toMap();

	// add every part
	QTextCursor textCursor = lastContentTextCursor();
	textCursor.beginEditBlock();

	QStringList parts = parseContent();
	int index = 0;
	foreach (QString part, parts)
	{
		if (index != 0)
			textCursor.insertBlock();

		bool isText = true;
		if (part.contains(QRegExp(kUuidReg)))
		{
			// image
			QString uuid = part.mid(1, part.length()-2);
			if (attachs.contains(uuid))
			{
				isText = false;
				QVariantMap attach = attachs[uuid].toMap();
				addImage(textCursor, attach);
			}
		}
		
		if (isText)
		{
			// text
			textCursor.insertText(part, charFormat);
		}

		++index;
	}

	textCursor.endEditBlock();
}

QStringList TextChatFrame::parseContent()
{
	QStringList parts;
	QString content = m_data[bean::kszBody].toString();
	int from = 0;
	int index = content.indexOf(QRegExp(kUuidReg), from);
	QString part;
	while (index != -1)
	{
		part = content.mid(from, index-from);
		if (!part.isEmpty())
			parts.append(part);

		part = content.mid(index, kUuidLength);
		parts.append(part);

		from += (index+kUuidLength);

		index = content.indexOf(QRegExp(kUuidReg), from);
	}
	part = content.mid(from);
	if (!part.isEmpty())
		parts.append(part);

	return parts;
}

void TextChatFrame::addImage(QTextCursor &textCursor, const QVariantMap &attach)
{
	bool sendImage = isSendAttach();

	QString attachUuid = attach[bean::kszUuid].toString();
	QString rawPath = attach[bean::kszPath].toString();
	QString path = ChatFrame::localPathFromRawPath(rawPath);

	QTextImageFormat imageFormat;
	imageFormat.setAnchorNames(QStringList() << attachUuid);

	if (sendImage)
	{
		// send image
		if (m_message4Js->isImageOk(rawPath))
		{
			// image is ok, show that image
			QVariantMap size = m_message4Js->getAutodisplaySize(attachUuid);
			int width = size[bean::kszWidth].toInt();
			int height = size[bean::kszHeight].toInt();

			QImage image(path);
			m_chatFrame->document()->addResource(QTextDocument::ImageResource, rawPath, image);

			imageFormat.setName(rawPath);
			imageFormat.setWidth(width);
			imageFormat.setHeight(height);
			imageFormat.setAnchor(true);
			imageFormat.setAnchorNames(QStringList() << attachUuid);
			imageFormat.setAnchorHref(QString("%1%2%3").arg(kszOpenImageCommand).arg(kszCommandSep).arg(rawPath));
			textCursor.insertImage(imageFormat);
		}
		else
		{
			// image is not ok, show error image
			rawPath = QString(":/images/errorBmp.png");
			imageFormat.setName(rawPath);
			textCursor.insertImage(imageFormat);
		}
	}
	else
	{
		// received image
		if ((attach[bean::kszResult].toString() == QString(bean::kszSuccessful)) &&
			m_message4Js->isImageOk(rawPath))
		{
			// image is downloaded ok, show that image
			QVariantMap size = m_message4Js->getAutodisplaySizeByUrl(rawPath);
			int width = size[bean::kszWidth].toInt();
			int height = size[bean::kszHeight].toInt();

			QImage image(path);
			m_chatFrame->document()->addResource(QTextDocument::ImageResource, rawPath, image);

			imageFormat.setName(rawPath);
			imageFormat.setWidth(width);
			imageFormat.setHeight(height);
			imageFormat.setAnchor(true);
			imageFormat.setAnchorNames(QStringList() << attachUuid);
			imageFormat.setAnchorHref(QString("%1%2%3").arg(kszOpenImageCommand).arg(kszCommandSep).arg(rawPath));
			textCursor.insertImage(imageFormat);
		}
		else
		{
			// image is not ok, show downloading image
			rawPath = QString(":/images/sendingBmp.png");
			imageFormat.setName(rawPath);
			textCursor.insertImage(imageFormat);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS VoiceChatFrame
VoiceChatFrame::VoiceChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent /*= 0*/)
: ChatFrame(frame, message4Js, parent)
{

}

VoiceChatFrame::~VoiceChatFrame()
{

}

void VoiceChatFrame::setData(const QVariantMap &data)
{
	ChatFrame::setData(data);

	QVariantMap attachs = data[bean::kszAttachs].toMap();
	QVariantMap attach = attachs.values().at(0).toMap();
	m_voiceUuid = attach[bean::kszUuid].toString();
}

void VoiceChatFrame::makeContent()
{
	QVariantMap attach = this->attach(m_voiceUuid);
	if (attach.isEmpty())
		return;

	QTextCursor textCursor = firstContentTextCursor();

	if (isSendAttach() || 
		attach[bean::kszResult].toString() == QString(bean::kszSuccessful))
	{
		QTextTableFormat tableFormat;
		tableFormat.setBorderStyle(QTextTableFormat::BorderStyle_None);
		tableFormat.setBorder(0);
		tableFormat.setCellSpacing(0);
		tableFormat.setCellPadding(0);
		QVector<QTextLength> columnWidths;
		columnWidths << QTextLength(QTextLength::FixedLength, 34);
		columnWidths << QTextLength(QTextLength::FixedLength, 72);
		tableFormat.setColumnWidthConstraints(columnWidths);
		QTextTable *voiceBody = textCursor.insertTable(1, 2, tableFormat);

		QTextTableCell leftCell = voiceBody->cellAt(0, 0);
		QTextTableCellFormat cellFormat;
		cellFormat.setBackground(QColor(108, 190, 255));
		cellFormat.setLeftPadding(2);
		cellFormat.setTopPadding(1);
		leftCell.setFormat(cellFormat);

		QTextTableCell rightCell = voiceBody->cellAt(0, 1);
		cellFormat.setBackground(Qt::white);
		cellFormat.setVerticalAlignment(QTextTableCellFormat::AlignMiddle);
		cellFormat.setLeftPadding(8);
		cellFormat.setTopPadding(0);
		rightCell.setFormat(cellFormat);

		textCursor = rightCell.firstCursorPosition();
		QString imageName = QString(":/html/images/audio_playing.gif");
		QImage audioPlayingImage(imageName);
		QTextCharFormat charFormat;
		charFormat.setObjectType(PM::AudioPlayingFormat);
		charFormat.setProperty(PM::AudioPlayingFrame, audioPlayingImage);
		textCursor.insertText(QString(QChar::ObjectReplacementCharacter), charFormat);

		textCursor.insertText(tr("   %1\"").arg(attach[bean::kszTime].toInt()));

		textCursor = leftCell.firstCursorPosition();
		imageName = QString(":/html/images/audio_play.png");
		QTextImageFormat imageFormat;
		imageFormat.setName(imageName);
		imageFormat.setBackground(QColor(108, 190, 255));
		imageFormat.setAnchor(true);
		imageFormat.setAnchorHref(QString("%1%2%3").arg(kszPlayCommand).arg(kszCommandSep).arg(m_voiceUuid));
		textCursor.insertImage(imageFormat);
	}
	else
	{
		// receiving
		QTextImageFormat imageFormat;
		imageFormat.setName(QString(":/html/images/loading.gif"));
		textCursor.insertImage(imageFormat);
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS AttachChatFrame
AttachChatFrame::AttachChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent /*= 0*/)
: ChatFrame(frame, message4Js, parent)
{
}

AttachChatFrame::~AttachChatFrame()
{

}

void AttachChatFrame::makeContent()
{
	QTextCursor textCursor = firstContentTextCursor();
	textCursor.insertText(tr("这是一条附件消息"));
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS TipChatFrame
TipChatFrame::TipChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent /*= 0*/)
: ChatFrame(frame, message4Js, parent)
{

}

TipChatFrame::~TipChatFrame()
{

}

void TipChatFrame::makeContent()
{
	QTextCursor textCursor = firstContentTextCursor();
	textCursor.insertText(tr("这是一条提示消息"));
}

//////////////////////////////////////////////////////////////////////////
// CLASS SecretTextChatFrame
SecretTextChatFrame::SecretTextChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent /*= 0*/)
: TextChatFrame(frame, message4Js, parent)
{
}

SecretTextChatFrame::~SecretTextChatFrame()
{
}

void SecretTextChatFrame::makeContent()
{
	QTextCursor textCursor = firstContentTextCursor();
	textCursor.insertText(tr("这是一条阅后即焚文本消息"));
}

//////////////////////////////////////////////////////////////////////////
// CLASS SecretVoiceChatFrame
SecretVoiceChatFrame::SecretVoiceChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent /*= 0*/)
: VoiceChatFrame(frame, message4Js, parent)
{

}

SecretVoiceChatFrame::~SecretVoiceChatFrame()
{

}

void SecretVoiceChatFrame::makeContent()
{
	QTextCursor textCursor = firstContentTextCursor();
	textCursor.insertText(tr("这是一条阅后即焚语音消息"));
}