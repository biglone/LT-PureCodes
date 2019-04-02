#ifndef DISCUSSTREEVIEW_H
#define DISCUSSTREEVIEW_H

#include <QTreeView>
#include "flickerable.h"

class DiscussModel;
class DiscussItemDelegate;
class DiscussItem;
class FlickerHelper;
class QActionGroup;

class DiscussTreeView : public QTreeView, public IFlickerable
{
	Q_OBJECT

public:
	explicit DiscussTreeView(QWidget *parent = 0);
	void setDiscussModel(DiscussModel *discussModel);
	DiscussModel *discussModel() const;
	void setFlickerHelper(FlickerHelper *flickerHelper);

public: // from IFlickerable
	bool containsFlickerItem(const QString &id, bean::MessageType msgType) const;
	void doUpdate();

signals:
	void chat(const QString &id);
	void multiMail();
	void viewPastChat(const QString &id);
	void exitDiscuss(const QString &id);
	void disbandDiscuss(const QString &id);
	void removeDiscussChat(const QString &id);
	void changeName(const QString &id);

public slots:
	void chat();
	void viewPastChat();
	void exitDiscuss();
	void disbandDiscuss();
	void contextMenu(const QPoint &position);
	void doubleClicked(const QModelIndex& index);
	void groupSetting(QAction *action);
	void changeName();
	void onDiscussAdded(const QString &id);

protected:
	void paintEvent(QPaintEvent *event);

private:
	DiscussItem *selectedItem();
	void setupActions();
	void setSkin();

private:
	DiscussModel *m_discussModel;
	DiscussItemDelegate *m_itemDelegate;

	QAction *m_chatAction;
	QAction *m_multiMailAction;
	QAction *m_viewPastChatsAction;
	QAction *m_exitDiscussAction;
	QAction *m_changeNameAction;

	QAction      *m_groupTip;
	QAction      *m_groupUntip;
	QActionGroup *m_groupActionGroup;
};

#endif // DISCUSSTREEVIEW_H
