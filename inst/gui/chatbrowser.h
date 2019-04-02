#ifndef CHATBROWSER_H
#define CHATBROWSER_H

#include <QTextBrowser>
#include "bean/MessageBody.h"
#include <QList>
#include <QMap>
#include <QTimer>
#include "autodisplaysizedelegate.h"

class ChatFrame;
class CMessage4Js;
class QTextFrame;
class TextChatFrame;
class QAction;

class ChatBrowser : public QTextBrowser, public AutoDisplaySizeDelegate
{
	Q_OBJECT

public:
	ChatBrowser(QWidget *parent);
	~ChatBrowser();

	CMessage4Js *message4Js() const;

	void scrollToBottom();
	bool isScrollAtBottom() const;

public: // from AutoDisplaySizeDelegate
	virtual QSize getAutoDisplaySize(const QSize &actualSize);

public slots:
	void appendMessage(const bean::MessageBody &msg);
	void insertMessageAtTop(const bean::MessageBody &msg);
	void setMessages(const bean::MessageBodyList &msgs);
	void removeAllMessages();

	void onContentsSizeChanged();

private slots:
	void onAnchorClicked(const QUrl &url);
	void onContextMenu(const QPoint &pt);

	// begin attach related
	void onDownloadChanged(const QString& rsUuid, const QString& fileName);
	void onProgress(const QString& rsUuid, int nPercent);
	void onStopped(const QString& rsUuid, const QString& rsOperator);

	void onAttachDownloadFinish(const QString &rsUuid);
	void onAutoDownloadFinish(const QString& rsUuid);
	void onAutoDisplayFinish(const QString &rsUuid, const QString &sPath, int nActWidth, int nActHeight, int nDispWidth, int nDispHeight);

	void onAttachDownloadError(const QString& rsUuid, const QString& rsOperator, const QString& rsError);
	void onAutoDownloadError(const QString& rsUuid, const QString& rsOperator, const QString& rsError);
	void onAutoDisplayError(const QString &rsUuid, const QString &rsOperator, const QString &rsError);

	void onAttachUploadFinish(const QString &rsUuid);
	void onAutoDisplayUploadFinish(const QString &rsUuid);
	void onAutoDownloadUploadFinish(const QString &rsUuid);
	void onUploadError(const QString &rsUuid, const QString &rsOperator, const QString &rsError);
	// end attach related

	void onImageResizeTimeout();

	void onCopyImage();
	void onSaveAsImage();

private:
	ChatFrame *makeMessage(const bean::MessageBody &msg, QTextFrame *msgFrame);
	void dealWithAttachs(const bean::MessageBody &msg, ChatFrame *chatFrame);
	void initDocumentResources();

private:
	bean::MessageBodyList                m_messages;
	QList<ChatFrame *>                   m_chatFrames;
	CMessage4Js                         *m_message4Js;
	
	QMap<QString, TextChatFrame *>       m_imageFrames;

	QTimer                               m_imageResizeTimer;
	QAction                             *m_copyImageAction;
	QAction                             *m_saveAsImageAction;
};

#endif // CHATBROWSER_H
