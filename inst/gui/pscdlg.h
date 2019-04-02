#ifndef _PSCDLG_H_
#define _PSCDLG_H_

#include <QModelIndex>
#include <QSystemTrayIcon>
#include <QEvent>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>
#include "framelessdialog.h"
#include "bean/bean.h"

class CChatDlg;         /// 对话窗口
class CDebugDlg;        /// 调试窗口(CTRL+SHIFT+D)
class PasswdModifyDlg;
class CAboutDialog;
class UnreadMessageBox;

class CSortFilterProxyModel;
class RosterModel;
class GroupModel;
class DiscussModel;
class OrgStructModel;
class LastContactModel;
class FlickerHelper;

class RosterTreeView;
class GroupListView;
class LastContactView;
class OrganizationTreeView;
class DiscussTreeView;

class ContactCard;

class ContactInfoManager;

class QxtGlobalShortcut;

class SnapShot;

class CChatDialog;
class CGroupDialog;
class DiscussDialog;

class GroupPanel;

class QPropertyAnimation;

class QTimer;

class SubscriptionDetail;
class SubscriptionMsg;

namespace bean {
	class DetailItem;
}

namespace Ui {
    class CPscDlg;
}

class CPscDlg : public FramelessDialog
{
	Q_OBJECT

public:
	explicit CPscDlg(QWidget *parent = 0);
    ~CPscDlg();

	static CPscDlg *instance();

	void init();

	void setRosterModel(RosterModel* model);
	void setOrgModel(OrgStructModel* model);
	void setGroupModel(GroupModel* model);
	void setDiscussModel(DiscussModel* model);
	void setLastContactModel(LastContactModel* model);

	void dockShow();

signals:
	void screenshotOk(const QString &imagePath);
	void screenshotCancel();
	void blackListChanged();

public slots:
	void openChat(const QString &id);
	void openChat(const QStringList &ids);
	void openGroupChat(const QString &id);
	void openGroupChat(const QStringList &ids);
	void openDiscussChat(const QString &id);
	void openDiscussChat(const QStringList &ids);
	void openAllUnreadChats(const QList<int> &msgTypes, const QStringList &ids);

	virtual void setSkin();

	void slot_statusChanged(int status);

	void slot_menu_exit();
	void slot_menu_switch_company();
	void slot_menu_logout();
	void slot_modify_passwd();

	void slot_receiveKick();

	void addFlickering(const QString &id, bean::MessageType msgType);
	void removeFlickering(const QString &id, bean::MessageType msgType);
	void clearFlickering();

	void setSignature();

	void viewContactInfo(const QString &id);
	void viewContactHistory(const QString &id);
	void viewGroupHistory(const QString &id);
	void viewDiscussHistory(const QString &id);

	void sendMail(const QString &id);

	void msgMultiSend(const QStringList &members, const QString &id = QString());

	void addFriendRequest(const QString &id, const QString &name);
	void onAddFriendOK(const QString &id, const QString &name, const QString &group);

	void openSubscriptionLastMsgDialog();
	void openSubscriptionMsg(const QString &subscriptionId);
	void openSubscriptionDetail(const QString &subscriptionId);
	void openSubscriptionHistory(const QString &subscriptionId);

	void onCompanyLoginFailed(const QString &desc);

protected:
	// 事件处理
	void keyPressEvent(QKeyEvent *e);
	void closeEvent(QCloseEvent *e);
	void moveEvent(QMoveEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void hideEvent(QHideEvent *e);
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private:
	// 初始化界面
	void InitUI();
	void InitViews();
	void InitStatusMenu();
	void InitPosition();
	void SavePosition();

private slots:
	void onMyInfoChanged(const QString &id);
	void onDetailChanged(const QString &id);
	void slotQuitAct();
	void slotLogoutAct();
	void slotRestoreAct();

	void slot_open_help();
	void slot_open_setting();
	void slot_open_about();
	void slot_open_update();
	void slot_system_mute_toggled();

	void slotMainMenuHide();

	void on_avatarWidget_clicked();

	void on_btnStatus_clicked();

	void on_btnMenu_toggled(bool checked);

	void on_btnSearch_clicked();

	void searchPeople();

	void searchSubscription();

	void on_btnSystemSettings_clicked();
	
	void on_btnMsgManager_clicked();

	void on_btnFileManager_clicked();

	void on_btnAddFriendList_clicked();

	void on_btnInterphone_clicked();

	void on_btnAppManage_clicked();

	void slot_statusMenu_triggered(QAction*);

	void slot_clickedMinimized();
	void slot_clickedClose();

	// edit filter related
	void editFilterChanged(const QString &filterText);
	void editFilterSelectItem(const QString &id, int source, const QString &wid);
	void mainPageChanged(int index);
	void editFilterGainFocus();

	// contact card
	void showContactCard(const QString &id, int posY);
	void hideContactCard();

	void setTopmost(bool topmost);

	void screenshot();
	void onScreenshotOk(const QString &imagePath);
	void onScreenshotCancel();

	void onChatDialogCreated(CChatDialog *chatDialog);
	void onGroupDialogCreated(CGroupDialog *groupDialog);
	void onDiscussDialogCreated(DiscussDialog *discussDialog);

	void onUnreadMsgCountChanged();
	void onUnreadMsgPreIgnoreAll();

	void modifyRosterFailed(const QString &errMsg, int actionType, const QStringList &ids, const QStringList &names, 
		const QStringList &groups, const QList<int> &modifies);
	void rosterModified(const QStringList &ids, const QStringList &names, const QStringList &groups, const QList<int> &modifies);

	void onDeleteFriendOK(const QString &id);
	void onDeleteFriendFailed(const QString &id, const QString &desc);
	void onUserDeleted(const QString &id);
	void onUserFrozen(const QString &id);

	// discuss related -- begin
	void createDiscussDialog(const QStringList &preAddUids = QStringList());
	void createInterphone(const QString &name, const QStringList &uids);
	void createDiscuss(const QString &name, const QStringList &uids);
	void exitDiscuss(const QString &id, bool force = false);
	void changeDiscussName(const QString &id);
	void addDiscussMembers(const QString &id, const QStringList &members);
	void disbandDiscuss(const QString &id);

	void onCreatedDiscuss(int handle, const QString &id);
	void onAddedMembers(int handle, const QString &id);
	void onQuitedDiscuss(int handle, const QString &id);
	void onQuitedDiscuss(const QString &id);
	void onDiscussNameChanged(int handle, const QString &id, const QString &name);
	void onDiscussError(int handle, int type, const QString &errmsg, const QString &discussId, const QString &discussName, const QStringList &members);
	void onNotifyDiscussChanged(const QString &id);
	void onDiscussNewAdded(const QString &id);

	void onDiscussChangeNotice(const QString &param);

	void onDiscussRemoved(const QString &discussId);

	void onDiscussKickFailed(const QString &discussId, const QString &uid, const QString &errMsg);
	void onDiscussKickOK(const QString &discussId, const QString &uid);
	void onDiscussDisbandFailed(const QString &discussId, const QString &errMsg);
	void onDiscussDisbandOK(const QString &discussId);
	// discuss related -- end

	void onGroupChangeNotice(const QString &param);

	void setShortcutKey();
	void showShortcutConflictDlg(int failedCount);
	void hideShortcutConflictDlg();
	void modifyShortcutKey();

	void dockIn();
	void dockOut(bool checkCursor = true);
	void onDockInFinished();
	void onDockOutFinished();
	void checkDock();

	void checkToClearAllMessages();
	void prepareForQuit();

	void onPasswdModified();
	void onValidateFailed();

	void onRosterAddNotice(int action, const QString &param);
	void onRosterAddNotice(int action, const QString &fromId, const QString &fromName, 
		const QString &toId, const QString &toName, const QString &sId, 
		const QString &group, const QString &message, bool read = false);
	void onRosterAddList();
	void setAddFriendUnhandleFlag(bool hasUnhandle);
	void onRosterAddResponded(const QString &param);

	void onSubscriptionSubscribed(const SubscriptionDetail &subscription, const SubscriptionMsg &msg);
	void onSubscriptionUnsubscribed(bool ok, const QString &subscriptionId);
	void onSubscriptionSubscribed(const QString &subscriptionId);
	void onSubscriptionUnsubscribed(const QString &subscriptionId);

	void removeSubscription(const QString &subscriptionId);

	void onSubscriptionUnreadMsgChanged(const QString &subscriptionId, int count);

	void openSubscriptionDialog();

	void addBlack(const QString &id);
	void removeBlack(const QString &id);
	void manageBlack();

	void setAvatar(int status);

	void removeGroupChat(const QString &groupId);
	void removeDiscussChat(const QString &discussId);

	void onAppButtonClicked();
	void setAppButtons();

	void onLinkItemClicked();

	void onConfigChanged(const QString &param);

	void setSilence(const QStringList &silenceList);

	void getGroupLogoFinished(const QString &gid, int version, const QImage &logo);

	void changeGroupCardNameFailed(const QString &gid, const QString &errMsg);

private:
	void doLogout();
	void doSwitchCompany(const QString &uid);

private:
	Ui::CPscDlg*               ui;

	QList<int>  m_discussHandles;
	QList<int>  m_quitHandles;
	QList<int>  m_addMemberHandles;
	QStringList m_newDiscussIds;
	QList<int>  m_interphoneDiscussHandles;
	QStringList m_interphoneDiscussId;

	QMenu                          m_mainMenu;
	QMenu*                         m_pStatusMenu;

	QPointer<PasswdModifyDlg>      m_pPasswdModifyDlg;
	QPointer<CAboutDialog>         m_pAboutDialog;
	QPointer<UnreadMessageBox>     m_pUnreadMessageBox;
	QPointer<ContactCard>          m_pContactCard;

	bool                       m_bClose;
	bool                       m_bKick;

	FlickerHelper   *m_pFlickerHelper;

	RosterTreeView       *m_rosterView;
	OrganizationTreeView *m_orgView;
	GroupListView        *m_groupView;
	DiscussTreeView      *m_discussView;
	LastContactView      *m_lastContactView;
	GroupPanel           *m_groupPanel;

	QScopedPointer<ContactInfoManager> m_pContactInfoManager;

	QxtGlobalShortcut *m_pTakeMsgShortcut;
	QxtGlobalShortcut *m_pScreenshotShortcut;
	SnapShot          *m_pScreenshot;

	bool                m_dockReady; // if is ready for a dock
	bool                m_dockState; // if is in dock state
	QPropertyAnimation *m_dockInAnimation;
	QPropertyAnimation *m_dockOutAnimation;
	bool                m_leftKeyPressed;
	QTimer             *m_dockCheckTimer;

	QList<QToolButton *> m_appButtons;
	QStringList          m_appButtonPathes;

	static CPscDlg *s_pscDlg;
};

#endif // _PSCDLG_H_
