#ifndef GROUPLISTVIEW_H
#define GROUPLISTVIEW_H

#include <QListView>
#include "flickerable.h"

class GroupModel;
class GroupItemDelegate;
class MucGroupItem;
class FlickerHelper;
class QActionGroup;

class GroupListView : public QListView, public IFlickerable
{
	Q_OBJECT

public:
	explicit GroupListView(QWidget *parent = 0);
	void setGroupModel(GroupModel *groupModel);
	GroupModel *groupModel() const;
	void setFlickerHelper(FlickerHelper *flickerHelper);

public: // from IFlickerable
	bool containsFlickerItem(const QString &id, bean::MessageType msgType) const;
	void doUpdate();

signals:
	void chat(const QString &id);
	void multiMail();
	void sendFile(const QString &id, const QStringList &fileNames);
	void viewPastChat(const QString &id);
	void removeGroupChat(const QString &id);

public slots:
	void chat();
	void viewPastChat();
	void sendFile();
	void contextMenu(const QPoint &position);
	void doubleClicked(const QModelIndex& index);
	void groupSetting(QAction *action);

protected:
	void paintEvent(QPaintEvent *e);

private:
	MucGroupItem *selectedItem();
	void setupActions();
	void setSkin();

private:
	GroupModel *m_groupModel;
	GroupItemDelegate *m_itemDelegate;
	QAction *m_chatAction;
	QAction *m_multiMailAction;
	QAction *m_sendFileAction;
	QAction *m_viewPastChatsAction;

	QAction      *m_groupTip;
	QAction      *m_groupUntip;
	QActionGroup *m_groupActionGroup;
};

#endif // GROUPLISTVIEW_H
