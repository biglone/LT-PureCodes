#include "chatbrowser.h"
#include "chatframe.h"
#include <QTextCursor>
#include <QTextFrame>
#include <QDebug>
#include "message4js.h"
#include "audioplayingobject.h"
#include "progressobject.h"
#include "chatinputdef.h"
#include <QScrollBar>
#include <QAction>
#include <QMenu>
#include "util/FileUtil.h"
#include "util/ImageUtil.h"
#include <QClipboard>
#include <QDesktopServices>
#include "settings/AccountSettings.h"
#include "pmessagebox.h"
#include <QApplication>
#include "Account.h"
#include "util/FileDialog.h"

ChatBrowser::ChatBrowser(QWidget *parent)
	: QTextBrowser(parent)
{
	setOpenLinks(false);
	setFocusPolicy(Qt::NoFocus);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setStyleSheet("border-image: none; border: none; selection-color: white; selection-background-color: rgb(49, 151, 252);");

	QObject *audioPlaying = new AudioPlayingObject();
	this->document()->documentLayout()->registerHandler(PM::AudioPlayingFormat, audioPlaying);

	QObject *progress = new ProgressObject();
	this->document()->documentLayout()->registerHandler(PM::ProgressFormat, progress);

	m_imageResizeTimer.setInterval(100);
	m_imageResizeTimer.setSingleShot(true);
	connect(&m_imageResizeTimer, SIGNAL(timeout()), this, SLOT(onImageResizeTimeout()));

	m_copyImageAction = new QAction(tr("复制"), this);
	connect(m_copyImageAction, SIGNAL(triggered()), this, SLOT(onCopyImage()));

	m_saveAsImageAction = new QAction(tr("另存为..."), this);
	connect(m_saveAsImageAction, SIGNAL(triggered()), this, SLOT(onSaveAsImage()));

	initDocumentResources();

	connect(this, SIGNAL(anchorClicked(QUrl)), this, SLOT(onAnchorClicked(QUrl)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onContextMenu(QPoint)));

	m_message4Js = new CMessage4Js(this);
	m_message4Js->setAutoDisplaySizeDelegate(this);
	connect(m_message4Js, SIGNAL(addMessageToTail(bean::MessageBody)), this, SLOT(appendMessage(bean::MessageBody)));
	connect(m_message4Js, SIGNAL(addMessageToTop(bean::MessageBody)), this, SLOT(insertMessageAtTop(bean::MessageBody)));
	connect(m_message4Js, SIGNAL(onDownloadChanged(QString, QString)), this, SLOT(onDownloadChanged(QString, QString)));
	connect(m_message4Js, SIGNAL(onProgress(QString, QString)), this, SLOT(onProgress(QString, QString)));
	connect(m_message4Js, SIGNAL(onStopped(QString, QString)), this, SLOT(onStopped(QString, QString)));
	connect(m_message4Js, SIGNAL(onAttachDownloadFinish(QString)), this, SLOT(onAttachDownloadFinish(QString)));
	connect(m_message4Js, SIGNAL(onAutoDownloadFinish(QString)), this, SLOT(onAutoDownloadFinish(QString)));
	connect(m_message4Js, SIGNAL(onAutoDisplayFinish(QString, QString, int, int, int, int)), 
		this, SLOT(onAutoDisplayFinish(QString, QString, int, int, int, int)));
	connect(m_message4Js, SIGNAL(onAttachDownloadError(QString, QString, QString)),
		this, SLOT(onAttachDownloadError(QString, QString, QString)));
	connect(m_message4Js, SIGNAL(onAutoDownloadError(QString, QString, QString)),
		this, SLOT(onAutoDownloadError(QString, QString, QString)));
	connect(m_message4Js, SIGNAL(onAutoDisplayError(QString, QString, QString)),
		this, SLOT(onAutoDisplayError(QString, QString, QString)));
	connect(m_message4Js, SIGNAL(onAttachUploadFinish(QString)), this, SLOT(onAttachUploadFinish(QString)));
	connect(m_message4Js, SIGNAL(onAutoDisplayUploadFinish(QString)), this, SLOT(onAutoDisplayUploadFinish(QString)));
	connect(m_message4Js, SIGNAL(onAutoDownloadUploadFinish(QString)), this, SLOT(onAutoDownloadUploadFinish(QString)));
	connect(m_message4Js, SIGNAL(onUploadError(QString, QString, QString)), this, SLOT(onUploadError(QString, QString, QString)));


	/*
	void onAttachUploadFinish(const QString &rsUuid);
	void onAutoDisplayUploadFinish(const QString &rsUuid);
	void onAutoDownloadUploadFinish(const QString &rsUuid);
	void onUploadError(const QString &rsUuid, const QString& rsOperator, const QString& rsError);
	*/

	/*
	QObject *audioPlaying = new AudioPlayingObject();
	m_textBrowser->document()->documentLayout()->registerHandler(AudioPlayingFormat, audioPlaying);

	QObject *progress = new ProgressObject();
	m_textBrowser->document()->documentLayout()->registerHandler(ProgressFormat, progress);

	QObject *secretCount = new SecretCountObject();
	m_textBrowser->document()->documentLayout()->registerHandler(SecretCountFormat, secretCount);

	m_imageTimer.setInterval(100);
	m_imageTimer.setSingleShot(true);
	connect(&m_imageTimer, SIGNAL(timeout()), this, SLOT(onImageTimeout()));

	m_attachTimer.setInterval(100);
	m_attachTimer.setSingleShot(true);
	connect(&m_attachTimer, SIGNAL(timeout()), this, SLOT(onAttachPercent()));

	m_secretDestroyTimer.setInterval(1000);
	m_secretDestroyTimer.setSingleShot(true);
	connect(&m_secretDestroyTimer, SIGNAL(timeout()), this, SLOT(onSecretDestroyTimeout()));

	m_selectContentAction = new QAction(QString("select message"), this);
	connect(m_selectContentAction, SIGNAL(triggered()), this, SLOT(selectTextContent()));

	initVoiceImageResource();

	initAttachImageResource();

	initSecretImageResource();

	addMoreBar();
	*/
}

ChatBrowser::~ChatBrowser()
{

}

CMessage4Js *ChatBrowser::message4Js() const
{
	return m_message4Js;
}

void ChatBrowser::scrollToBottom()
{
	QScrollBar *vScrollBar = verticalScrollBar();
	int maxVal = vScrollBar->maximum();
	vScrollBar->setValue(maxVal);
}

bool ChatBrowser::isScrollAtBottom() const
{
	QScrollBar *vScrollBar = verticalScrollBar();
	int maxVal = vScrollBar->maximum();
	if (maxVal == vScrollBar->value())
		return true;
	else
		return false;
}

QSize ChatBrowser::getAutoDisplaySize(const QSize &actualSize)
{
	QSize sz = actualSize;
	int maxWidth = this->width();
	int maxHeight = this->height();
	maxWidth = (int)((maxWidth-80)*0.8);
	maxHeight = maxHeight - 120;
	if (sz.width() > maxWidth || sz.height() > maxHeight)
		sz.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
	return sz;
}

void ChatBrowser::appendMessage(const bean::MessageBody &msg)
{
	bool atBottom = isScrollAtBottom();

	QTextCursor textCursor = this->textCursor();
	textCursor.movePosition(QTextCursor::End);
	QTextFrameFormat frameFormat;
	frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
	frameFormat.setBorderBrush(Qt::white);
	frameFormat.setBorder(1);
	QTextFrame *msgFrame = textCursor.insertFrame(frameFormat);

	ChatFrame *chatFrame = makeMessage(msg, msgFrame);
	if (!chatFrame)
	{
		qWarning() << Q_FUNC_INFO << "message can't make chat frame: " << msg.toJson();
		return;
	}

	m_chatFrames.append(chatFrame);
	m_messages.append(msg);

	if (msg.isSend() && !msg.sync())
	{
		scrollToBottom();
	}
	else if (atBottom) // received message at bottom
	{
		scrollToBottom();
	}

	dealWithAttachs(msg, chatFrame);
}

void ChatBrowser::insertMessageAtTop(const bean::MessageBody &msg)
{
	QTextCursor textCursor = this->textCursor();
	textCursor.movePosition(QTextCursor::Start);
	QTextFrameFormat frameFormat;
	frameFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
	frameFormat.setBorderBrush(Qt::white);
	frameFormat.setBorder(1);
	QTextFrame *msgFrame = textCursor.insertFrame(frameFormat);

	ChatFrame *chatFrame = makeMessage(msg, msgFrame);
	if (!chatFrame)
	{
		qWarning() << Q_FUNC_INFO << "message can't make chat frame: " << msg.toJson();
		return;
	}

	m_chatFrames.insert(0, chatFrame);
	m_messages.insert(0, msg);

	dealWithAttachs(msg, chatFrame);
}

void ChatBrowser::setMessages(const bean::MessageBodyList &msgs)
{
	removeAllMessages();

	foreach (bean::MessageBody msg, msgs)
	{
		appendMessage(msg);
	}
}

void ChatBrowser::removeAllMessages()
{
	this->clear();
	m_messages.clear();
	m_chatFrames.clear();
	m_imageFrames.clear();
}

void ChatBrowser::onContentsSizeChanged()
{
	m_imageResizeTimer.stop();
	m_imageResizeTimer.start();
}

void ChatBrowser::onAnchorClicked(const QUrl &url)
{
	QString anchorStr = url.toString();
	int index = anchorStr.indexOf(kszCommandSep);
	if (index == -1)
	{
		qDebug() << Q_FUNC_INFO << "invalid command: " << anchorStr;
		return;
	}

	QString cmd = anchorStr.left(index);
	QString param = anchorStr.mid(index+1);
	if (cmd == kszUserIdCommand)
	{
		// clicked uid
		m_message4Js->uidClicked(param);
	}
	else if (cmd == kszAvatarCommand)
	{
		// click avatar
		m_message4Js->avatarClicked(param);
	}
	else if (cmd == kszOpenImageCommand)
	{
		// click image
		QUrl url = QUrl::fromEncoded(param.toUtf8());
		m_message4Js->openUrl(url);
	}
	else
	{

	}
}

void ChatBrowser::onContextMenu(const QPoint &pt)
{
	QString anchorStr = this->anchorAt(pt);
	qDebug() << Q_FUNC_INFO << anchorStr;

	if (anchorStr.startsWith(kszOpenImageCommand))
	{
		int index = anchorStr.indexOf(kszCommandSep);
		if (index == -1)
			return;

		QString imagePath = anchorStr.mid(index+1);
		QString localPath = ChatFrame::localPathFromRawPath(imagePath);
		m_copyImageAction->setData(localPath);
		m_saveAsImageAction->setData(localPath);
		QMenu *menu = new QMenu();
		menu->addAction(m_copyImageAction);
		menu->addAction(m_saveAsImageAction);
		QPoint menuPt = this->mapToGlobal(pt);
		menu->exec(menuPt);
		delete menu;
		menu = 0;
	}
}

void ChatBrowser::onDownloadChanged(const QString& rsUuid, const QString& fileName)
{

}

void ChatBrowser::onProgress(const QString& rsUuid, int nPercent)
{

}

void ChatBrowser::onStopped(const QString& rsUuid, const QString& rsOperator)
{

}

void ChatBrowser::onAttachDownloadFinish(const QString &rsUuid)
{

}

void ChatBrowser::onAutoDownloadFinish(const QString& rsUuid)
{

}

void ChatBrowser::onAutoDisplayFinish(const QString &rsUuid, const QString &sPath,
									  int nActWidth, int nActHeight, int nDispWidth, int nDispHeight)
{
	if (m_imageFrames.contains(rsUuid))
	{
		TextChatFrame *imageFrame = m_imageFrames[rsUuid];
		imageFrame->setImageDownloadOK(rsUuid, sPath, nActWidth, nActHeight, nDispWidth, nDispHeight);
	}
}

void ChatBrowser::onAttachDownloadError(const QString& rsUuid, const QString& rsOperator, const QString& rsError)
{

}

void ChatBrowser::onAutoDownloadError(const QString& rsUuid, const QString& rsOperator, const QString& rsError)
{

}

void ChatBrowser::onAutoDisplayError(const QString &rsUuid, const QString & /*rsOperator*/, const QString & /*rsError*/)
{
	if (m_imageFrames.contains(rsUuid))
	{
		TextChatFrame *imageFrame = m_imageFrames[rsUuid];
		imageFrame->setImageDownloadError(rsUuid);
	}
}

void ChatBrowser::onAttachUploadFinish(const QString &rsUuid)
{

}

void ChatBrowser::onAutoDisplayUploadFinish(const QString &rsUuid)
{
	if (m_imageFrames.contains(rsUuid))
	{
		TextChatFrame *imageFrame = m_imageFrames[rsUuid];
		imageFrame->setImageUploadOK(rsUuid);
	}
}

void ChatBrowser::onAutoDownloadUploadFinish(const QString &rsUuid)
{

}

void ChatBrowser::onUploadError(const QString & rsUuid, const QString & /*rsOperator*/, const QString & /*rsError*/)
{
	if (m_imageFrames.contains(rsUuid))
	{
		TextChatFrame *imageFrame = m_imageFrames[rsUuid];
		imageFrame->setImageUploadError(rsUuid);
	}
}

void ChatBrowser::onImageResizeTimeout()
{
	// resize all ok images
	foreach (QString attachUuid, m_imageFrames.keys())
	{
		TextChatFrame *imageFrame = m_imageFrames[attachUuid];
		QSize actSize = imageFrame->imageActualSize(attachUuid);
		if (actSize.isValid())
		{
			QSize dispSize = this->getAutoDisplaySize(actSize);
			imageFrame->setImageDisplaySize(attachUuid, dispSize);
		}
	}
}

void ChatBrowser::onCopyImage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString sPath = action->data().toString();
	if (sPath.isEmpty())
		return;

	QWidget *window = this->window();
	if (!FileUtil::fileExists(sPath))
	{
		PMessageBox::warning(window, tr("提示"), tr("此文件不存在，可能被删除或者被移动到其他位置。"));
		return;
	}

	QImage img = ImageUtil::readImage(sPath);
	if (img.isNull())
	{
		qWarning("onCopyActionTriggered: image is null");
		return;
	}

	QMimeData *mimeData = new QMimeData();
	mimeData->setImageData(img);
	mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(sPath));
	QApplication::clipboard()->setMimeData(mimeData);
}

void ChatBrowser::onSaveAsImage()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString fileName = action->data().toString();
	if (fileName.isEmpty())
		return;

	QWidget *window = this->window();
	if (!FileUtil::fileExists(fileName))
	{
		PMessageBox::warning(window, tr("提示"), tr("此文件不存在，可能被删除或者被移动到其他位置。"));
		return;
	}

	QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
		saveDir = accountSettings->getCurDir();

	QFileInfo fi(fileName);
	QString saveName = QString("%1/%2").arg(saveDir).arg(fi.fileName());
	QString newFileName = FileDialog::getImageSaveFileName(this, tr("保存图像"), saveName);
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

ChatFrame *ChatBrowser::makeMessage(const bean::MessageBody &msg, QTextFrame *msgFrame)
{
	ChatFrame *chatFrame = 0;
	bean::MessageExt ext = msg.ext();
	if (ext.type() == bean::MessageExt_Secret)
	{
		QList<bean::AttachItem> attachs = msg.attachs();
		if (attachs.isEmpty())
		{
			chatFrame = new SecretTextChatFrame(msgFrame, m_message4Js, this);
		}
		else
		{
			foreach (bean::AttachItem attach, attachs)
			{
				if (attach.transferType() == bean::AttachItem::Type_AutoDownload)
				{
					chatFrame = new SecretVoiceChatFrame(msgFrame, m_message4Js, this);
					break;
				}
				else
				{
					chatFrame = new SecretTextChatFrame(msgFrame, m_message4Js, this);
					break;
				}
			}
		}
	}
	else
	{
		QList<bean::AttachItem> attachs = msg.attachs();
		if (attachs.isEmpty())
		{
			chatFrame = new TextChatFrame(msgFrame, m_message4Js, this);
		}
		else
		{
			foreach (bean::AttachItem attach, attachs)
			{
				if (attach.transferType() == bean::AttachItem::Type_Default)
				{
					chatFrame = new AttachChatFrame(msgFrame, m_message4Js, this);
					break;
				}
				else if (attach.transferType() == bean::AttachItem::Type_AutoDownload)
				{
					chatFrame = new VoiceChatFrame(msgFrame, m_message4Js, this);
					break;
				}
				else
				{
					chatFrame = new TextChatFrame(msgFrame, m_message4Js, this);
					break;
				}
			}
		}
	}

	if (chatFrame)
	{
		QVariantMap dataMap = msg.toJson();
		chatFrame->setData(dataMap);
		chatFrame->makeChatFrame();
	}

	return chatFrame;
}

void ChatBrowser::dealWithAttachs(const bean::MessageBody &msg, ChatFrame *chatFrame)
{
	/*
	if ((obj.method === null || ( typeof obj.method === "string" && obj.method === "Method_Send")) && obj.sync != 1) {
		$.each(obj.attachs, function(n,value) {
			if (value.result != "successful") {
				if(value.type != undefined && (value.type == "auto-display" || value.type == "auto-download")){	
					startAutoUpload(value.uuid);
				} else {
					startupload(value.uuid);
				}
			}
		});
	} 
	else {
		// 自动下载图片或语音
		$.each(obj.attachs, function(n,value) {	
			if (value.result != "successful") {
				if(value.type != undefined && (value.type == "auto-display" || value.type == "auto-download")){	
					startAutoDownload(value.uuid);
				}
			} else if (value.type == "auto-display" && value.result == "successful" && !Message4Js.isImageOk(value.path)) {
				startAutoDownload(value.uuid);
			}
		});
	}
	*/
	QList<bean::AttachItem> attachs = msg.attachs();
	foreach (bean::AttachItem attach, attachs)
	{
		QString attachUuid = attach.uuid();
		bean::AttachItem::TransferResult result = (bean::AttachItem::TransferResult)attach.transferResult();
		if (attach.transferType() == bean::AttachItem::Type_Default)
		{
		}
		else if (attach.transferType() == bean::AttachItem::Type_AutoDownload)
		{
		}
		else // Type_AutoDisplay
		{
			TextChatFrame *textChatFrame = static_cast<TextChatFrame *>(chatFrame);
			m_imageFrames.insert(attachUuid, textChatFrame);
			if (textChatFrame->isSendAttach())
			{
				if (result != bean::AttachItem::Transfer_Successful)
					m_message4Js->startAutoUpload(attachUuid);
			}
			else
			{
				if (result != bean::AttachItem::Transfer_Successful)
				{
					m_message4Js->startAutoDownload(attachUuid);
				}
				else
				{
					QVariantMap attachMap = attach.toVariantMap();
					QString rawPath = attachMap[bean::kszPath].toString();
					if (!m_message4Js->isImageOk(rawPath))
						m_message4Js->startAutoDownload(attachUuid);
				}

			}
		}
	}
}

void ChatBrowser::initDocumentResources()
{
	/*
	QStringList allRes;
	allRes << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/"
		   << ":/html/images/";
	foreach (QString res, allRes)
	{
		QImage image(res);
		this->document()->addResource(QTextDocument::ImageResource, res, image);
	}
	*/
}
