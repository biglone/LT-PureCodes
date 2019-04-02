#ifndef SUBSCRIPTIONMSGDIALOG_H
#define SUBSCRIPTIONMSGDIALOG_H

#include "framelessdialog.h"
#include <QMenu>
#include <QList>
#include "subscriptionmsg.h"
#include <QMap>
#include "webview.h"

namespace Ui {class SubscriptionMsgDialog;};
class SubscriptionMsg4Js;
class SubscriptionMenu;
class QPushButton;
class QAction;
class MiniSplitter;
class SubscriptionMsgWebPage;

class SubscriptionMsgDialog : public FramelessDialog, public WebViewMenuDelegate
{
	Q_OBJECT

public:
	SubscriptionMsgDialog(const QString &id, QWidget *parent = 0);
	~SubscriptionMsgDialog();

	QString subscriptionId() const;

	void getMessages();
	void setMessages(const QList<SubscriptionMsg> &msgs);
	void appendMessage(const SubscriptionMsg &msg);

public: // From WebViewMenuDelegate
	void addMenuAction(QMenu *menu, const QWebElement &webElement);

Q_SIGNALS:
	void openSubscriptionDetail(const QString &subscriptionId);
	void cleanup();
	void initUIComplete();
	void openUrl(const QString &subscriptionId, const QString &url);
	void clickMenu(const QString &subscriptionId, const QString &key);
	void openTitle(const QString &subscriptionId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &subscriptionId, const QString &urlStr, const QString &name);
	void viewMaterial(const QString &uid);

public slots:
	void setSkin();

protected:
	void keyPressEvent(QKeyEvent *event);

private slots:
	void on_btnSend_clicked();
	void showTip(const QString &tip);
	void on_btnSendSetting_clicked();
	void slot_sendshortkey_changed(QAction* action);
	void on_icon_clicked();
	void on_btnExit_clicked();
	void fetchHistoryMsg();
	void populateJavaScriptWindowObject();
	void onMessage4JsLoadSucceeded();
	void onMaximizeStateChanged(bool isMaximized);
	void onSubscriptionMenuChanged(const QString &subscriptionId);
	void onSubscriptionMenuButtonToggled(bool checked);
	void onSubscriptionSubMenuClicked();
	void downloadImage();
	void onUserChanged(const QString &uid);
	void onLogoChanged(const QString &subscriptionId);
	void showMenuBar();
	void hideMenuBar();
	void onSubscriptionMenuHide();
	void onGotHistoryMsg(qint64 seq, const QList<SubscriptionMsg> &msgs);

private:
	void initUI();
	void setSubscriptionMenu();
	void triggerMenuAction(const QAction *action);

private:
	Ui::SubscriptionMsgDialog *ui;

	QString m_subscriptionId;

	QMenu   m_sendShortcutMenu;

	quint64                m_oldestInnerId;

	SubscriptionMsg4Js    *m_message4Js;

	QAction *m_downloadImageAction;

	MiniSplitter *m_chatSplitter;

	SubscriptionMsgWebPage *m_webPage;

	QMenu                                    m_subscriptionMenu;
	QList<QPushButton *>                     m_menuButtons; // all buttons on menu bar
	QList<QWidget *>                         m_menuWidgets; // all widgets on menu bar
	QMap<QPushButton *, QAction *>           m_menuActions; // menu actions
	QMap<QPushButton *, QList<QAction *>>    m_subMenuActions; // sub-menu actions

	QList<qint64> m_historySequences;
	qint64        m_messagesSequence;
};

#endif // SUBSCRIPTIONMSGDIALOG_H
