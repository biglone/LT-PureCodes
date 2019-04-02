#ifndef MESSAGEPANNEL_H
#define MESSAGEPANNEL_H

#include <QWidget>
#include <QTimer>
#include <QMenu>

#include "bean/MessageBody.h"
#include "emotion/EmotionUtil.h"
#include "webview.h"
#include "messageresenddelegate.h"

class CMessage4Js;
class MiniSplitter;
class CChatInputBox;
class QAction;
class SecretSwitchTip;
class ChatWebPage;

namespace Ui {class MessagePannel;};

class MessageSendDelegate
{
public:
	virtual void sendMessage(const bean::MessageBody &msgBody) = 0;
};

class MessagePannel : public QWidget, 
	                  public WebViewMenuDelegate, 
					  public WebViewImageDragDelegate, 
					  public EmotionCallback,
					  public MessageResendDelegate
{
	Q_OBJECT

public:
	enum MessageMode
	{
		Normal,
		Secret
	};

public:
	MessagePannel(QWidget *parent = 0);
	virtual ~MessagePannel();

public:
	void init(bean::MessageType type, const QString &id, const QString &name);

	bean::MessageType type() const;
	QString id() const;
	QString name() const;
	void setName(const QString &name);

	void setMsgSendDelegate(MessageSendDelegate *msgSendDelegate);

	void layoutPannel();

	void setSupportSecretMessage();

	void scrollMessageToBottom();

	void updateChatAvatar(const QString &uid);

	void updateChatName();

public:
	void insertWidgetToToolbar(int index, QWidget *widget);

	QToolButton *historyButton() const;
	QToolButton *msgSettingButton() const;
	QToolButton *shakeButton() const;
	CChatInputBox *chatInput() const;

public: // From WebViewMenuDelegate
	void addMenuAction(QMenu *menu, const QWebElement &webElement);

public: // From WebViewImageDragDelegate
	bool canDragImage(const QWebElement &imageElement);

public: // From MessageResendDelegate
	bool resendMessageOfSequence(const QString &seq);

public:
	// callback emotion
	virtual QObject *instance();
	virtual QWidget *instanceWindow();
	virtual void onEmotionSelected(bool defaultEmotion, const QString &emotionId);
	virtual void emotionClosed();

public:
	virtual void onMessage(const bean::MessageBody &rBody, bool history = false, bool firstHistory = false, bool showAt = true);
	virtual void clearMessages();
	virtual void showMoreMsgTip();
	virtual void closeMoreMsgTip();
	virtual void onMoreMsgFinished();
	virtual void showMoreHistoryMsgTip();
	virtual void showAutoTip(const QString &tip);

public slots:
	virtual void setSkin();

	void slot_screenshot_ok(const QString &imagePath);
	void slot_screenshot_cancel();

	void appendTipMessage(const QString &timeStr, const QString &msgText, const QString &level, 
		                  const QString &action = QString(), const QString &param = QString());

	void anchorAtMsg(const QString &atId);

	bool resendFailedMessage(const QString &seq);

	void showTip(const QString &tip);

	// -- begin interphone related
	void addInterphone();
	void openInterphone();
	void quitInterphone();
	// -- end   interphone related

	void appendHistorySeparator();

	void onWithdrawOK(bean::MessageType chatType, const QString &toId, const QString &fromId, 
		const QString &timeStamp, const QString &tipText, const bean::MessageBody &origMsg);
	void onWithdrawFailed(bean::MessageType chatType, const QString &toId, 
		const QString &fromId, const QString &timeStamp);
	
Q_SIGNALS:
	void initUIComplete();
	void doScreenshot();
	void cleanup();
	void chat(const QString &uid);
	void sendMail(const QString &uid);
	void multiMail();
	void viewMaterial(const QString &uid);
	void shakeDialog();
	void inputChanged();
	void recordStart();
	void messageToSend();
	void closeRequest();
	void sendSecretMessageRead(const QString &stamp);
	void recvSecretMessageRead(const QString &stamp);
	void fetchHistoryMsg();
	void openHistoryMsg();
	void messageSent(const QString &seq, const QString &stamp);
	void messageWithdrawed(const QString &stamp, const QString &tipText);
	void doClearMessages();

protected:
	void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event);

private Q_SLOTS:
	void on_switchSecret_toggled(bool checked);

	void on_btnFace_clicked(bool checked);

	void on_btnAttach_clicked();
	void insertAttachFiles();
	void insertAttachDirs();

	void on_btnImage_clicked();

	void on_btnShake_clicked();

	void on_btnRecord_clicked();

	void on_btnSend_clicked();

	void on_tBtnScreenshot_clicked();

	void on_tBtnCleanup_clicked();

	void on_btnSendSetting_clicked();

	void on_btnExit_clicked();

	void slot_sendshortkey_changed(QAction* action);

	// void fetchHistoryMsg();

	void onScreenShotMenuAboutToShow();

	void hideToScreenShotToggled(bool checked);

	// -- begin record related
	void onRecordFinished(int recordId, const QString &fileName);
	void onRecordCanceled(int recordId);
	void onRecordStartError(int recordId, const QString &desc);
	void onRecordTimeElapsed(int recordId, int timeInMs);
	void onSendRecord();
	void onCancelRecord();
	// -- end   record related

	// -- begin interphone related
	void onInterphoneStarted(const QString &interphoneId);
	void onInterphoneFinished(const QString &interphoneId);
	void onInterphoneChanged(const QString &interphoneId, int attachType, const QString &attachId);
	void onAddInterphoneFinished(bool ok, const QString &interphoneId);
	// void onQuitInterphoneFinished(bool ok, const QString &interphoneId);
	void onInterphoneCleared();
	// -- end   interphone related

	void onCopyActionTriggered();
	void onSaveActionTriggered();
	void addFavoriteEmotion();
	void withdrawMessage();
	void copyMessage();
	void forwardMessage();

	// secret message
	void onSendSecretMessageRead(const QString &uid, const QString &stamp);
	void onRecvSecretMessageRead(const QString &uid, const QString &stamp);

	void onMessageSent(bean::MessageType msgType, const QString &id, const QString &seq, const QString &stamp);

	void onShortcutKeyChanged();

	void onGotMessageOfStamp(qint64 seq, const QString &stamp, const bean::MessageBody &msg);

private:
	void initUI();

	bool needShowAtMsg(const bean::MessageBody &rBody) const;
	void showAtMsg(const bean::MessageBody &rBody, bool top);
	void hideAtMsg();
	bool isAtMsgShown() const;
	
	void showRecordBar();
	void hideRecordBar();

	void checkIfInInterphone();

	bool checkBlackList(const QString &op);

	void sendAttachMessages(const QStringList &rsFiles);

	void showMessageWithdrawTip(const QString &tipText);
	void doMessageWithdraw(const QString &stamp, const bean::MessageBody &msg);

	void doMessageCopy(const bean::MessageBody &msg);
	void doMessageForward(const bean::MessageBody &msg);
	void showMessageForwardTip(const QString &tipText);

	void showAddFavoritEmotionOKTip();

private:
	Ui::MessagePannel       *ui;

	bean::MessageType	     m_type;
	QString				     m_id;
	QString				     m_name;

	QMenu				     m_sendShortcutMenu;

	QMenu				     m_screenShotMenu;

	QMenu				     m_attachMenu;

	CMessage4Js*		     m_pMessage4js;

	MiniSplitter*		     m_chatSplitter;

	QTimer				     m_sizeChangeTimer;

	bool				     m_bIsFirstRecvMsg;

	QTimer*                  m_shakingIntervalTimer;

	MessageSendDelegate     *m_msgSendDelegate;

	int                      m_recordId;

	QAction                 *m_copyAction;
	QAction                 *m_saveAction;
	QAction                 *m_addFavoriteEmotion;
	QAction                 *m_messageWithdrawAction;
	QAction                 *m_messageCopyAction;
	QAction                 *m_messageForwardAction;

	MessageMode              m_messageMode;

	SecretSwitchTip         *m_secretSwitchTip;

	ChatWebPage             *m_chatWebPage;

	QMap<qint64, QString>    m_withdrawStamps;
	QMap<qint64, QString>    m_copyStamps;
	QMap<qint64, QString>    m_forwardStamps;
};

#endif // MESSAGEPANNEL_H
