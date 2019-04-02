#ifndef UNIONCHATDIALOG_H
#define UNIONCHATDIALOG_H

#include "FramelessDialog.h"
#include "bean/bean.h"
#include "unionchatdelegate.h"
#include <QList>
#include <QRect>
#include <QTimer>
#include <QHBoxLayout>
#include "minisplitter.h"

namespace Ui {class UnionChatDialog;};
class QStandardItemModel;
class QStandardItem;
class ChatBaseDialog;
class QModelIndex;
struct ITaskbarList;
class QTimer;
class ChatProxy;

extern const int kChatListMsgTypeRole;
extern const int kChatListMsgIdRole;
extern const int kChatListMsgCountRole;

class UnionChatDialog : public FramelessDialog, public UnionChatDelegate
{
	Q_OBJECT

public:
	UnionChatDialog(QWidget *parent = 0);
	~UnionChatDialog();

	void clearChatUnread(ChatBaseDialog *chatWidget);

public slots:
	void setSkin();
	void triggerMaximize();
	void insertChatAtTop(bean::MessageType msgType, const QString &id, 
		                 ChatBaseDialog *chatWidget, bool makeCurrent = true);
	void insertMultiChatAtTop(const QString &id, const QStringList &members, 
		                 ChatBaseDialog *chatWidget, bool makeCurrent = true);
	bool removeChat(QWidget *chatWidget);
	void setCurrentChat(QWidget *chatWidget);
	void flashChat(QWidget *chatWidget);
	bool hasChatWidget(QWidget *chatWidget);

public: // functions from class UnionChatDelegate
	virtual bool isMaximumState() const;
	virtual void chatWidthChanged(int width);
	virtual void chatMinimumWidthChanged(int minWidth);

protected:
	bool nativeEvent(const QByteArray &eventType, void *message, long *result);
	bool winEvent(MSG *message, long *result);
	void closeEvent(QCloseEvent *ev);
	void keyPressEvent(QKeyEvent *event);
	void changeEvent(QEvent *ev);

private slots:
	void onMaximizeStateChanged(bool isMaximized);
	void onListItemClicked(const QModelIndex &index);
	void removeChat();
	void chatIconChanged(const QPixmap &pixmap);
	void chatNameChanged(const QString &name);
	void activateFromTaskbar();
	void closeFromTaskbar();
	void onCurrentChatChanged(int index);
	void onChatRemoved(int index);
	void chatUnreadMessageCountChanged(int count);
	void setChatThumbnail(int maxWidth, int maxHeight);
	void setChatLivePreview();
	void startShake();
	void shakingTimeout();
	void hideListFinished();
	void clearCurrentUnreadMessage();
	void insertMimeData(const QModelIndex &index, const QMimeData *source);

private:
	void initUI();
	void initShake();
	QStandardItem * createListItem(bean::MessageType msgType, const QString &id);
	QStandardItem * createMultiChatListItem(const QString &id, const QStringList &members);
	void addListItemWidget(QStandardItem *item);
	void flashTaskBar(QWidget *w);
	void unflashTaskBar(QWidget *w);
	void connectChatWidget(ChatBaseDialog *chatWidget);
	void setTaskbarIconAndText(QWidget *w, QStandardItem *item);
	void setTaskbarIcon(QWidget *w, const QPixmap &pixmap);
	void setTaskbarText(QWidget *w, const QString &text);
	bool insertTaskbarTab(QWidget *insertWidget, QWidget *beforeWidget);
	bool removeTaskbarTab(QWidget *widget);
	bool setActiveTaskbarTab(QWidget *widget);
	void clearUnreadMessage(ChatBaseDialog *chatWidget);
	bool flashNextUnread();
	void showList();
	void hideList();
	ChatProxy *createChatProxy();

private:
	Ui::UnionChatDialog *ui;

	QStandardItemModel *m_chatListModel;

	unsigned int        m_taskbarBtnCreatedMsg;
	ITaskbarList       *m_pTl3;

	int                 m_previousIndex;

	QTimer              m_clearUnreadTimer;

	QWidget            *m_lastFlashWidget;

	// about shaking
	QTimer             *m_shakingTimer;
	int                 m_shakingCount;
	QList<QPoint>       m_shakingPosList;
	QRect               m_shakingFrameBak;

	bool                m_blockingClose;

	QHBoxLayout        *m_mainLayout;
	MiniSplitter       *m_splitter;

};

#endif // UNIONCHATDIALOG_H
