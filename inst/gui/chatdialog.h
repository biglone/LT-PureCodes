#ifndef _CHATDAILOG_H_
#define _CHATDAILOG_H_

#include <QWidget>
#include <QList>
#include "chatbasedialog.h"
#include "bean/MessageBody.h"

namespace bean {class DetailItem;};
namespace Ui {class CChatDialog;};

class QTimer;
class CChatDialog : public ChatBaseDialog
{
	Q_OBJECT
public:
	explicit CChatDialog(const QString &rsUid, QWidget *parent = 0);
	~CChatDialog();

public slots:
	void setSkin();

	void slot_screenshot_ok(const QString &imagePath);
	void slot_screenshot_cancel();

	void onBlackListChanged();

	void onSessionClosed();

	virtual void insertMimeData(const QMimeData *source);
	virtual void loadHistoryMessages(int count);
	virtual void fetchHistoryMessages();
	virtual void fetchMoreMessages();

	virtual void clearMessages();

Q_SIGNALS:
	void viewContactInfo(const QString &id);
	void doScreenshot();
	void sendMail(const QString &id);
	void viewMaterial(const QString &uid);
	void addFriendRequest(const QString &id, const QString &name);
	void removeBlack(const QString &id);
	void createDiscuss(const QStringList &preAddUids);

protected:
	void closeEvent(QCloseEvent *e);
	void keyPressEvent(QKeyEvent *event);

public: // functions from CChatInterface in ChatBaseDialog
	virtual void appendSendMessage(const bean::MessageBody &rBody);
	virtual void onMessage(const bean::MessageBody &rBody, bool history = false, bool firstHistory = false);
	virtual void showAutoTip(const QString &tip);

    virtual void onSessionInvite(const QString &sid);
	virtual void startVideo();

private: // functions from ChatBaseDialog
	virtual void addUnreadMessageCount(int addCount = 1);
	virtual int unreadMessageCount() const;
	virtual void clearUnreadMessageCount();

	virtual void setMaximizeState(bool maximizeState);
	virtual bool isExpanded() const;
	virtual int unExpandedWidth() const;
	virtual bool canClose();
	virtual void onUnionStateChanged();
	
	virtual void showMoreMsgTip();
	virtual void closeMoreMsgTip();
	virtual void onMoreMsgFinished();
	virtual void showMoreHistoryMsgTip();
	virtual void appendHistorySeparator();
	virtual void focusToEdit();

private Q_SLOTS:
	void onUserChanged(const QString &id);

	void onUserPresenceChanged(const QString &id);

	void onUserPresenceChanged();

	void on_icon_clicked();

	void on_title_clicked();

	void on_btnVideo_clicked();

	void on_btnPtt_clicked();

	void on_btnInterphone_clicked();
	
	void onbtnHistoryMsgClicked(bool checked = true);

	void onbtnHistoryMsgToggled(bool checked);

	void onSessionVideoSetup();

	void shakingTimeout();

	void setSideTabWidgetVisible(bool visible, bool removeAllTabs = true);

	void slotSideTabCloseRequest(int index);

	void onMaximizeStateChanged(bool isMaximized);

	void onBtnMoreMessageClicked();

	void onHistoryTabWidgetCurrentChanged(int index);

	void sendMail();

	void inviteToNewDiscuss();

	void addFriend();

	void onRosterChanged();

	void onShakeDialog();

	void onInputChanged();

	void onRecordStart();

	void onMessageToSend();

	void onInputShowTimeout();

	void onInputTipRecved(const QString &from, const QString &to, const QString &action);

	void onSpeakTipRecved(const QString &from, const QString &to, const QString &action);

	void on_blackLabel_clicked();

	void onLoadHistoryMessagesFinished(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs);

private:
	void InitUI(const QString &uid);

	void initShake();

	void startShake();

	void setTitle(bean::DetailItem* pItem);

	void setIcon(bean::DetailItem* pItem);
	
	void setUserMsg(bean::DetailItem* pItem);

	QString getName();

	void showSessionWidget();
	
	void hideSessionWidget(bool enableSession = true);

	void initInputTimer();

	void startShowInputTip(const QString &tipText);

	void stopShowInputTip();

private:
	Ui::CChatDialog* ui;

	QTimer        *m_shakingTimer;
	int            m_shakingCount;
	QList<QPoint>  m_shakingPosList;
	QRect          m_shakingFrameBak;

	QTimer        *m_inputChangeTimer;
	QTimer        *m_inputShowTimer;
	int            m_inputShowIndex;

	qint64         m_fetchHistoryMsgId; 

	friend class SessionWidget;
};

#endif // _CHATDAILOG_H_
