#ifndef DISCUSSDIALOG_H
#define DISCUSSDIALOG_H

#include <QMenu>
#include <QList>
#include "chatbasedialog.h"
#include "bean/MessageBody.h"
#include "groupmemberview.h"

class CSortFilterProxyModel;

namespace Ui {class DiscussDialog;};

class DiscussDialog : public ChatBaseDialog, public GroupMemberViewMenuDelegate
{
	Q_OBJECT

public:
	explicit DiscussDialog(const QString &id, QWidget *parent = 0);
	virtual ~DiscussDialog();

	QString id() const;

	void createInterphone();

	QString discussName() const;

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

	void quitDiscuss(const QString &discussId, bool force);
	void addDiscussMembers(const QString &discussId, const QStringList &memberIds);

	void removeDiscussChat(const QString &id);

protected:
	void closeEvent(QCloseEvent *e);
	void keyPressEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *e);

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

private: // functions from GroupMemberViewMenuDelegate
	virtual void appendMenuItem(QMenu *menu, const QString &uid);

private Q_SLOTS:
	void onbtnHistoryMsgClicked(bool checked = true);

	void onbtnHistoryMsgToggled(bool check);

	void onBtnGroupSettingClicked();

	void onMsgSettingsMenuAboutToShow();

	void setSideTabWidgetVisible(bool visible, bool removeAllTabs = true);

	void slotSideTabCloseRequest(int index);

	void onMaximizeStateChanged(bool isMaximized);

	void onBtnMoreMessageClicked();

	void onHistoryTabWidgetCurrentChanged(int index);

	void setMemberCount();

	void groupSetting(QAction *action);

	// discuss
	void on_btnAddMembers_clicked();

	void on_btnQuitDiscuss_clicked();

	void onMemberChanged();

	void on_labelCreator_clicked();

	void onDiscussInfoChanged(const QString &discussId);

	void on_btnInterphone_clicked();

	void onLoadHistoryMessagesFinished(qint64 seq, int curPage, int maxPage, const bean::MessageBodyList &msgs);

	void kickMember();

	void onUserChanged(const QString &uid);

	void changeCardName(const QString &uid, const QString &cardName);

	void changeDiscussName();

	void selfAdaptiveTitleEditLength();
	
private:
	void InitUI(const QString &id);

	void setDiscussInfo(const QString &id);

	void setDiscussCreateInfo(const QString &id);

	void setDiscussLogo(const QPixmap &logo);

	void initFont();

	void initMsgSettingActions();

	void initAtCompleter();

private:
	Ui::DiscussDialog *ui;

	CSortFilterProxyModel* m_pGroupItemProxyModel;

	QAction      *m_groupTip;
	QAction      *m_groupUntip;
	QActionGroup *m_groupActionGroup;
	QMenu         m_groupSettingMenu;
	int           m_quitHandle;

	qint64        m_fetchHistoryMsgId; 

	QAction      *m_kickMember;
};

#endif // DISCUSSDIALOG_H
