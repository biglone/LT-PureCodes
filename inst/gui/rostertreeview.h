#ifndef ROSTERTREEVIEW_H
#define ROSTERTREEVIEW_H

#include <QTreeView>
#include "flickerhelper.h"
#include "rosterdelegate.h"

class RosterModel;
class RosterDelegate;
class QStandardItem;
class QAction;
class FlickerHelper;

class RosterTreeView : public QTreeView, public IFlickerable
{
	Q_OBJECT

public:
	RosterTreeView(QWidget *parent = 0);
	~RosterTreeView();

	QStandardItem* selectedItem();
	void setRosterModel(RosterModel *rosterModel);
	RosterModel *rosterModel() const;
	void setFlickerHelper(FlickerHelper *flickerHelper);

public: // from IFlickerable
	bool containsFlickerItem(const QString &id, bean::MessageType msgType) const;
	void doUpdate();

Q_SIGNALS:
	void showCard(const QString &id, int posY);
	void hideCard();
	void chat(const QString &id);
	void sendMail(const QString &id);
	void viewPastChat(const QString &id);
	void viewMaterial(const QString &id);
	void msgMultiSend(const QStringList &ids);
	void addBlack(const QString &id);
	void removeBlack(const QString &id);
	void manageBlack();
	void openSubscription();

public slots:
	void doubleClicked(const QModelIndex &index);
	void clicked(const QModelIndex &index);
	void contextMenu(const QPoint &position);
	void chat();
	void sendMail();
	void viewMaterial();
	void viewPastChat();
	void removeRoster();
	void msgMultiSend();
	void addBlack();
	void removeBlack();

	void onPresenceChanged(const QString &id);
	void onPresenceCleared();

protected:
	void mouseReleaseEvent(QMouseEvent *event);
	bool event(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

private slots:
	void onRosterSetFinished();
	void switchAvatarType();

private:
	void setupActions();
	void setSkin();
	bool isLogined();
	bool checkLogined(const QString &operation);

private:
	// roster actions
	QAction                   *m_chat;
	QAction                   *m_sendMail;
	QAction                   *m_viewPastChats;
	QAction                   *m_viewMaterial;
	QAction                   *m_removeRoster;
				              
	QAction                   *m_addBlack;
	QAction                   *m_removeBlack;
	QAction                   *m_manageBlack;
				              
	QAction                   *m_msgMultSend;
				              
	QModelIndex                m_clickedIndex;
	RosterModel               *m_rosterModel;
	RosterDelegate            *m_itemDelegate;

	QAction                   *m_avatarTypeSwitchAction;
	RosterDelegate::AvatarType m_avatarType;
};

#endif // ROSTERTREEVIEW_H
