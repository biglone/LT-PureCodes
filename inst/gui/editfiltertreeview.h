#ifndef EDITFILTERTREEVIEW_H
#define EDITFILTERTREEVIEW_H

#include <QTreeView>
#include <QModelIndex>
#include "KeyDelegate.h"
#include "editfilterwidget.h"

class QStandardItemModel;
class EditFilterItemDelegate;
class QAction;
class RosterBaseItem;

class EditFilterTreeView : public QTreeView, public KeyDelegate
{
	Q_OBJECT

public:
	EditFilterTreeView(QWidget *parent);
	~EditFilterTreeView();

	void setSelectMode(EditFilterWidget::SelectMode mode);
	EditFilterWidget::SelectMode selectMode() const;

	void setEditFilterModel(QStandardItemModel *sourceModel);
	QStandardItemModel *editFilterModel() const;
	void selectFirstItem();

	void setRosterGroup(RosterBaseItem *rosterGroup);
	void setOsGroup(RosterBaseItem *osGroup);
	void setGroupGroup(RosterBaseItem *groupGroup);
	void setDiscussGroup(RosterBaseItem *discussGroup);
	void setSubscriptionGroup(RosterBaseItem *subscriptionGroup);

	void setRosterGroupInfo(const QString &groupText, bool viewAll);
	void setOsGroupInfo(const QString &groupText, bool viewAll);
	void setGroupGroupInfo(const QString &groupText, bool viewAll);
	void setDiscussGroupInfo(const QString &groupText, bool viewAll);
	void setSubscriptionGroupInfo(const QString &groupText, bool viewAll);

	RosterBaseItem *rosterGroup() const;
	RosterBaseItem *osContactGroup() const;
	RosterBaseItem *groupGroup() const;
	RosterBaseItem *discussGroup() const;
	RosterBaseItem *subscriptionGroup() const;

	void addRosterItem(RosterBaseItem *rosterItem);
	void addOsContactItem(RosterBaseItem *osContactItem);
	void addGroupItem(RosterBaseItem *groupItem);
	void addDiscussItem(RosterBaseItem *discussItem);
	void addSubscriptionItem(RosterBaseItem *subscriptionItem);

	void clearAll();
	void clearRoster();
	void clearOsContact();
	void clearGroup();
	void clearDiscuss();
	void clearSubscription();

	void setExpandAll();

public: // from KeyDelegate
	void upKeyPressed();
	void downKeyPressed();
	void returnKeyPressed();

public slots:
	void doubleClicked(const QModelIndex &index);
	void clicked(const QModelIndex &index);
	void contextMenu(const QPoint &position);

signals:
	void chat(const QString &id);
	void groupChat(const QString &id);
	void discussChat(const QString &id);
	void subscriptionChat(const QString &id);
	void viewMaterial(const QString &id);

	/* source: 0-roster 1-organization 2-group 3-discuss 4-subscription*/
	void selectSearchItem(const QString &id, int source, const QString &wid = QString());

	void viewAll(int type);

protected:
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

private slots:
	void chat();
	void groupChat();
	void discussChat();
	void subscriptionChat();
	void viewMaterial();
	void selectSearchItem();
	void clickAction(const QModelIndex &index);

private:
	void setupActions();

private:
	QStandardItemModel     *m_sourceModel;
	EditFilterItemDelegate *m_itemDelegate;
	QModelIndex             m_clickedIndex;

	RosterBaseItem         *m_rosterGroup;
	RosterBaseItem         *m_osContactGroup;
	RosterBaseItem         *m_groupGroup;
	RosterBaseItem         *m_discussGroup;
	RosterBaseItem         *m_subscriptionGroup;

	QAction                *m_chatAction;
	QAction                *m_groupAction;
	QAction                *m_discussAction;
	QAction                *m_subscriptionAction;
	QAction                *m_selectAction;
	QAction                *m_viewMaterialAction;

	EditFilterWidget::SelectMode m_selectMode;
};

#endif // EDITFILTERTREEVIEW_H
