#ifndef CHATBASEDIALOG_H
#define CHATBASEDIALOG_H

#include <QWidget>
#include <QPointer>
#include "unionchatdelegate.h"
#include "bean/MessageBody.h"

extern const int     kPageHistoryMessageCount;
extern const int     kMaxHistoryMessageCount;

class QMimeData;
class ChatProxy;

class ChatBaseDialog : public QWidget
{
	Q_OBJECT

public:
	enum UnionState
	{
		Single,
		Union
	};

public:
	ChatBaseDialog(QWidget *parent = 0);
	virtual ~ChatBaseDialog();

	void setUnionChatDelegate(UnionChatDelegate *unionChatDelegate);
	UnionChatDelegate *unionChatDelegate() const;

	virtual void addUnreadMessageCount(int addCount = 1);
	virtual int unreadMessageCount() const;
	virtual void clearUnreadMessageCount();

	void setChatProxy(ChatProxy *proxy);
	ChatProxy *chatProxy() const;
	HWND chatProxyWinId() const;

	UnionState unionState() const;
	void setUnionState(UnionState unionState);

	void setMoreCount(int count);
	int moreCount() const;
	bool canFetchMore() const;
	void addMessageCount(int addCount = 1);
	int allMessageCount() const;
	void clearMessageCount();
	void setHistoryMsgEndTime(const QString &endTime);
	QString historyMsgEndTime() const;
	void setHistoryMsgPageIndex(int pageIndex);
	int historyMsgPageIndex() const;
	void setMoreMessages(const bean::MessageBodyList &msgs);
	bean::MessageBodyList moreMessages() const;

	void setBlockingClose(bool block);
	bool isBlockingClose() const;

Q_SIGNALS:
	void requestClose();
	void requestMinimized();
	void requestMaximized();

	void chatIconChanged(const QPixmap &pixmap);
	void chatNameChanged(const QString &name);

	void activateFromTaskbar();
	void closeFromTaskbar();

	void setThumbnail(int maxWidth, int maxHeight);
	void setLivePreview();

	void chatUnreadMessageCountChanged(int count);

	void removeChat(QWidget *chatWidget);

	void shaking();

public slots:
	void closeChat();
	virtual void insertMimeData(const QMimeData * /*source*/) {}
	virtual void loadHistoryMessages(int /*count*/) {}
	virtual void fetchHistoryMessages() {}
	virtual void fetchMoreMessages() {}
	virtual void clearMessages() {}

public:
	virtual void setMaximizeState(bool maximizeState) = 0;
	virtual bool isExpanded() const = 0;
	virtual int unExpandedWidth() const = 0;
	virtual bool canClose() { return true; }

	virtual void showMoreMsgTip() {}
	virtual void closeMoreMsgTip() {}
	virtual void onMoreMsgFinished() {}

	virtual void appendHistorySeparator() {}

	virtual void focusToEdit() {}

	//////////////////////////////////////////////////////////////////////////
	// original interface from CChatInterface
	virtual void appendSendMessage(const bean::MessageBody &rBody) { Q_UNUSED(rBody); }
	virtual void onMessage(const bean::MessageBody &rBody, bool history = false, bool firstHistory = false) 
	{ Q_UNUSED(rBody); Q_UNUSED(history); Q_UNUSED(history); Q_UNUSED(firstHistory); }

	virtual void showAutoTip(const QString &tip) { Q_UNUSED(tip); }

	// session related
    virtual void onSessionInvite(const QString &sid) { Q_UNUSED(sid); }
	virtual void startVideo() {}

protected:
	virtual void onUnionStateChanged() {}

protected:
	UnionChatDelegate       *m_unionChatDelegate;
	int                      m_unreadMessageCount;
	QPointer<ChatProxy>      m_chatProxy;
	UnionState               m_unionState;

	int                      m_moreCount;
	int                      m_messageCount;
	QString                  m_historyMsgEndTime;
	int                      m_historyMsgPageIndex;
	bean::MessageBodyList    m_moreMessages;

	bool                     m_blockingClose;
};

class ChatProxy : public QWidget
{
Q_OBJECT

public:
	ChatProxy(QWidget *parent = 0);
	virtual ~ChatProxy();

	static bool isSupportTaskbarApi();

Q_SIGNALS:
	void proxyActivated();
	void proxyClose();
	void setThumbnail(int maxWidth, int maxHeight);
	void setLivePreview();

protected:
	bool nativeEvent(const QByteArray &eventType, void *message, long *result);
	bool winEvent(MSG *message, long *result);

private:
	int m_thumbnailWidth;
	int m_thumbnailHeight;
};

#endif // CHATBASEDIALOG_H
