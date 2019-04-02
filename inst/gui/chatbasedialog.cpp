#include "chatbasedialog.h"
#include <QDebug>
#include <Windows.h>
#include <dwmapi.h>
#include <QSysInfo>

const int     kPageHistoryMessageCount = 20;
const int     kMaxHistoryMessageCount  = 60;

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS ChatBaseDialog
ChatBaseDialog::ChatBaseDialog(QWidget *parent)
	: QWidget(parent)
	, m_unionChatDelegate(0)
	, m_unreadMessageCount(0)
	, m_unionState(Union)
	, m_messageCount(0)
	, m_moreCount(0)
	, m_historyMsgPageIndex(INT_MAX)
	, m_blockingClose(false)
{

}

ChatBaseDialog::~ChatBaseDialog()
{
	if (!m_chatProxy.isNull())
		m_chatProxy.data()->close();
}

void ChatBaseDialog::setUnionChatDelegate(UnionChatDelegate *unionChatDelegate)
{
	m_unionChatDelegate = unionChatDelegate;
}

UnionChatDelegate *ChatBaseDialog::unionChatDelegate() const
{
	return m_unionChatDelegate;
}

void ChatBaseDialog::addUnreadMessageCount(int addCount /*= 1*/)
{
	m_unreadMessageCount += addCount;
	
	emit chatUnreadMessageCountChanged(m_unreadMessageCount);
}

int ChatBaseDialog::unreadMessageCount() const
{
	return m_unreadMessageCount;
}

void ChatBaseDialog::clearUnreadMessageCount()
{
	m_unreadMessageCount = 0;
}

void ChatBaseDialog::setChatProxy(ChatProxy *proxy)
{
	if (proxy)
	{
		connect(proxy, SIGNAL(proxyActivated()), this, SIGNAL(activateFromTaskbar()), Qt::UniqueConnection);
		connect(proxy, SIGNAL(proxyClose()), this, SIGNAL(closeFromTaskbar()), Qt::UniqueConnection);
		connect(proxy, SIGNAL(setThumbnail(int, int)), this, SIGNAL(setThumbnail(int, int)), Qt::UniqueConnection);
		connect(proxy, SIGNAL(setLivePreview()), this, SIGNAL(setLivePreview()), Qt::UniqueConnection);
	}
	m_chatProxy = proxy;
}

ChatProxy *ChatBaseDialog::chatProxy() const
{
	return m_chatProxy.data();
}

HWND ChatBaseDialog::chatProxyWinId() const
{
	HWND hwnd = NULL;
	if (!m_chatProxy.isNull())
		hwnd = (HWND)(m_chatProxy.data()->winId());
	return hwnd;
}

ChatBaseDialog::UnionState ChatBaseDialog::unionState() const
{
	return m_unionState;
}

void ChatBaseDialog::setUnionState(UnionState unionState)
{
	if (m_unionState != unionState)
	{
		m_unionState = unionState;
		onUnionStateChanged();
	}
}

void ChatBaseDialog::setMoreCount(int count)
{
	if (count < 0)
		count = 0;
	m_moreCount = count;
}

int ChatBaseDialog::moreCount() const
{
	return m_moreCount;
}

bool ChatBaseDialog::canFetchMore() const
{
	if (m_moreCount > 0)
		return true;

	if (m_messageCount >= kMaxHistoryMessageCount)
	{
		return false;
	}
	else
	{
		if (m_historyMsgPageIndex <= 1)
			return false;
		else 
			return true;
	}
}

void ChatBaseDialog::addMessageCount(int addCount /*= 1*/)
{
	m_messageCount += addCount;
}

int ChatBaseDialog::allMessageCount() const
{
	return m_messageCount;
}

void ChatBaseDialog::clearMessageCount()
{
	m_messageCount = 0;
}

void ChatBaseDialog::setHistoryMsgEndTime(const QString &endTime)
{
	m_historyMsgEndTime = endTime;
}

QString ChatBaseDialog::historyMsgEndTime() const
{
	return m_historyMsgEndTime;
}

void ChatBaseDialog::setHistoryMsgPageIndex(int pageIndex)
{
	m_historyMsgPageIndex = pageIndex;
}

int ChatBaseDialog::historyMsgPageIndex() const
{
	return m_historyMsgPageIndex;
}

void ChatBaseDialog::setMoreMessages(const bean::MessageBodyList &msgs)
{
	m_moreMessages = msgs;
}

bean::MessageBodyList ChatBaseDialog::moreMessages() const
{
	return m_moreMessages;
}

void ChatBaseDialog::closeChat()
{
	emit removeChat(this);
}

void ChatBaseDialog::setBlockingClose(bool block)
{
	m_blockingClose = block;
}

bool ChatBaseDialog::isBlockingClose() const
{
	return m_blockingClose;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS ChatProxy
ChatProxy::ChatProxy(QWidget *parent /*= 0*/) : QWidget(parent), m_thumbnailWidth(64), m_thumbnailHeight(64)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	Qt::WindowFlags flags = windowFlags();
	flags = (flags | Qt::Window);
	flags = (flags | (~(Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint|Qt::WindowMaximizeButtonHint)));
	setWindowFlags(flags);

	setFixedSize(0, 0);
}

ChatProxy::~ChatProxy()
{

}

bool ChatProxy::isSupportTaskbarApi()
{
	if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ChatProxy::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	const QByteArray kWindowsEventTypeId("windows_generic_MSG");
	if (eventType == kWindowsEventTypeId)
	{
		MSG *msg = (MSG *)message;
		return winEvent(msg, result);
	}
	return false;
}

bool ChatProxy::winEvent(MSG *message, long *result)
{
	Q_UNUSED(result);

	if (!message)
		return false;

	if (message->message == WM_ACTIVATE)
	{
		if (LOWORD(message->wParam) != WA_INACTIVE)
		{
			emit proxyActivated();
		}

		*result = 0;
		return true;
	}
	else if (message->message == WM_CLOSE)
	{
		emit proxyClose();

		*result = 0;
		return true;
	}

	if (isSupportTaskbarApi())
	{
		if (message->message == WM_DWMSENDICONICTHUMBNAIL)
		{
			// first set thumbnail
			m_thumbnailWidth = HIWORD(message->lParam);
			m_thumbnailHeight = LOWORD(message->lParam);
			emit setThumbnail(m_thumbnailWidth, m_thumbnailHeight);
		}
		else if (message->message == WM_DWMSENDICONICLIVEPREVIEWBITMAP)
		{
			// show preview
			emit setLivePreview();

			// refresh thumbnail
			emit setThumbnail(m_thumbnailWidth, m_thumbnailHeight);
		}
	}

	return false;
}
