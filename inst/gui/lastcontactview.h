#ifndef LASTCONTACTVIEW_H
#define LASTCONTACTVIEW_H

#include <QListView>
#include "flickerable.h"

class LastContactModel;
class LastContactItem;
class LastContactItemDelegate;
class FlickerHelper;
class QActionGroup;

class LastContactView : public QListView, public IFlickerable
{
	Q_OBJECT

public:
	LastContactView(QWidget *parent = 0);
	void setLastContactModel(LastContactModel *lastContactModel);
	LastContactModel *lastContactModel() const;
	void setFlickerHelper(FlickerHelper *flickerHelper);

public: // from IFlickerable
	bool containsFlickerItem(const QString &id, bean::MessageType msgType) const;
	void doUpdate();

signals:
	void chat(const QString &id);
	void sendMail(const QString &id);
	void groupChat(const QString &id);
	void discussChat(const QString &id);
	void msgMultiSend(const QStringList &members, const QString &id);
	void sendFile(const QString &id, const QStringList &fileNames);
	void groupSendFile(const QString &id, const QStringList &fileNames);
	void discussSendFile(const QString &id, const QStringList &filenames);
	void viewPastChat(const QString &id);
	void groupViewPastChat(const QString &id);
	void discussViewPastChat(const QString &id);
	void viewMaterial(const QString &id);
	void groupViewMaterial(const QString &id);
	void discussViewMaterial(const QString &id);
	void addFriendRequest(const QString &id, const QString &name);
	void removeGroupChat(const QString &id);
	void removeDiscussChat(const QString &id);
	void showCard(const QString &id, int posY);
	void hideCard();
	void multiMail();
	void removeAllChats();
	void openSubscriptionMsg(const QString &subscriptionId);
	void openSubscriptionHistory(const QString &subscriptionId);
	void openSubscriptionDetail(const QString &subscriptionId);
	void addBlack(const QString &id);
	void removeBlack(const QString &id);
	void manageBlack();

public slots:
	void chat();
	void sendMail();
	void viewPastChat();
	void sendFile();
	void viewMaterial();
	void removeChat();
	void addFriend();
	void contextMenu(const QPoint &position);
	void doubleClicked(const QModelIndex& index);
	void groupSetting(QAction *action);

	void onStartInterphone(const QString &interphoneId);
	void onFinishInterphone(const QString &interphoneId);
	void onAddInterphone(bool ok, const QString &interphoneId);
	void onQuitInterphone(bool ok, const QString &interphoneId);
	void onInterphoneCleared();

	void addBlack();
	void removeBlack();

protected:
	bool event(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void paintEvent(QPaintEvent *event);

private:
	LastContactItem *selectedItem();
	void setupActions();
	void setSkin();

private:
	LastContactModel *m_lastContactModel;
	LastContactItemDelegate *m_itemDelegate;
	QAction *m_chatAction;
	QAction *m_sendMailAction;
	QAction *m_multiMailAction;
	QAction *m_sendFileAction;
	QAction *m_viewPastChatsAction;
	QAction *m_viewMaterialAction;
	QAction *m_removeChatAction;
	QAction *m_addFriendAction;
	QAction *m_removeAllChatsAction;
	QAction *m_addBlackAction;
	QAction *m_removeBlackAction;
	QAction *m_manageBlackAction;

	QAction      *m_groupTip;
	QAction      *m_groupUntip;
	QActionGroup *m_groupActionGroup;
};

#endif // LASTCONTACTVIEW_H
