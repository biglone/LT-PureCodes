#ifndef CGROUPDIALOG_H
#define CGROUPDIALOG_H

#include <QMenu>
#include <QList>
#include "framelessdialog.h"
#include "chatbasedialog.h"
#include "bean/MessageBody.h"

class CSortFilterProxyModel;

namespace Ui {class CGroupDialog;};

class CGroupDialog : public ChatBaseDialog
{
	Q_OBJECT
public:
	explicit CGroupDialog(const QString& rsGid, QWidget *parent = 0);
	~CGroupDialog();

	QString groupName() const;

public slots:
	virtual void setSkin();

	void slot_screenshot_ok(const QString &imagePath);
	void slot_screenshot_cancel();

	virtual void insertMimeData(const QMimeData *source);
	virtual void loadHistoryMessages(int count);
	virtual void fetchHistoryMessages();
	virtual void fetchMoreMessages();

	virtual void clearMessages();

Q_SIGNALS:
	void doScreenshot();
	void chat(const QString &uid);
	void sendMail(const QString &uid);
	void multiMail();
	void viewMaterial(const QString &uid);
	void addFriendRequest(const QString &uid, const QString &name);
	void removeGroupChat(const QString &id);

protected:
	void closeEvent(QCloseEvent *e);
	void keyPressEvent (QKeyEvent *event);

private: // functions from CChatInterface in ChatBaseDialog
	virtual void appendSendMessage(const bean::MessageBody &rBody);
	virtual void onMessage(const bean::MessageBody &rBody, bool history = false, bool firstHistory = false);
	virtual void showAutoTip(const QString &tip);

private: // functions from ChatBaseDialog
	virtual void addUnreadMessageCount(int addCount = 1);
	virtual int unreadMessageCount() const;
	virtual void clearUnreadMessageCount();

	virtual void setMaximizeState(bool maximizeState);
	virtual bool isExpanded() const;
	virtual int unExpandedWidth() const;
	virtual void onUnionStateChanged();

	virtual void showMoreMsgTip();
	virtual void closeMoreMsgTip();
	virtual void onMoreMsgFinished();
	virtual void showMoreHistoryMsgTip();

	virtual void appendHistorySeparator();

	virtual void focusToEdit();

private Q_SLOTS:
	void onbtnHistoryMsgClicked(bool checked = true);

	void onbtnHistoryMsgToggled(bool checked);

	void onBtnGroupSettingClicked();

	void onMsgSettingsMenuAboutToShow();

	void setSideTabWidgetVisible(bool visible, bool removeAllTabs = true);

	void slotSideTabCloseRequest(int index);

	void onMaximizeStateChanged(bool isMaximized);

	void onBtnMoreMessageClicked();

	void onHistoryTabWidgetCurrentChanged(int index);

	void setMemberCount();

	void groupSetting(QAction *action);

	void on_btnInterphone_clicked();

	void onLoadHistoryMessagesFinished(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs);

	void onUserChanged(const QString &uid);

	void onMemberChanged();

	void changeCardName(const QString &uid, const QString &cardName);

private:
	void InitUI(const QString &id);

	void initMsgSettingActions();

	void initAtCompleter();

private:
	Ui::CGroupDialog*  ui;

	QAction      *m_groupTip;
	QAction      *m_groupUntip;
	QActionGroup *m_groupActionGroup;
	QMenu         m_groupSettingMenu;

	qint64         m_fetchHistoryMsgId; 
};

#endif // CGROUPDIALOG_H
