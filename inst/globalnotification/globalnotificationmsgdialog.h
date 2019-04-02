#ifndef GLOBALNOTIFICATIONMSGDIALOG_H
#define GLOBALNOTIFICATIONMSGDIALOG_H

#include "framelessdialog.h"
#include <QMenu>
#include <QList>
#include "globalnotificationmsg.h"
#include <QMap>
#include "webview.h"

namespace Ui {class GlobalNotificationMsgDialog;};
class GlobalNotificationMsg4Js;
class GlobalNotificationMenu;
class QPushButton;
class QAction;
class MiniSplitter;
class GlobalNotificationMsgWebPage;

class GlobalNotificationMsgDialog : public FramelessDialog, public WebViewMenuDelegate
{
	Q_OBJECT

public:
	GlobalNotificationMsgDialog(const QString &id, QWidget *parent = 0);
	~GlobalNotificationMsgDialog();

	QString globalNotificationId() const;

	void getMessages();
	void setMessages(const QList<GlobalNotificationMsg> &msgs);
	void appendMessage(const GlobalNotificationMsg &msg);

public: // From WebViewMenuDelegate
	void addMenuAction(QMenu *menu, const QWebElement &webElement);

Q_SIGNALS:
	void openGlobalNotificationDetail(const QString &globalNotificationId);
	void cleanup();
	void initUIComplete();
	void openUrl(const QString &globalNotificationId, const QString &url);
	void clickMenu(const QString &globalNotificationId, const QString &key);
	void openTitle(const QString &globalNotificationId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &globalNotificationId, const QString &urlStr, const QString &name);
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
	void onGlobalNotificationMenuChanged(const QString &globalNotificationId);
	void onGlobalNotificationMenuButtonToggled(bool checked);
	void onGlobalNotificationSubMenuClicked();
	void downloadImage();
	void onUserChanged(const QString &uid);
	void onLogoChanged(const QString &globalNotificationId);
	void showMenuBar();
	void hideMenuBar();
	void onGlobalNotificationMenuHide();
	void onGotHistoryMsg(qint64 seq, const QList<GlobalNotificationMsg> &msgs);

private:
	void initUI();
	void setGlobalNotificationMenu();
	void triggerMenuAction(const QAction *action);

private:
	Ui::GlobalNotificationMsgDialog *ui;

	QString m_globalNotificationId;

	QMenu   m_sendShortcutMenu;

	quint64                m_oldestInnerId;

	GlobalNotificationMsg4Js    *m_message4Js;

	QAction *m_downloadImageAction;

	MiniSplitter *m_chatSplitter;

	GlobalNotificationMsgWebPage *m_webPage;

	QMenu                                    m_globalNotificationMenu;
	QList<QPushButton *>                     m_menuButtons; // all buttons on menu bar
	QList<QWidget *>                         m_menuWidgets; // all widgets on menu bar
	QMap<QPushButton *, QAction *>           m_menuActions; // menu actions
	QMap<QPushButton *, QList<QAction *>>    m_subMenuActions; // sub-menu actions

	QList<qint64> m_historySequences;
	qint64        m_messagesSequence;
};

#endif // GLOBALNOTIFICATIONMSGDIALOG_H
