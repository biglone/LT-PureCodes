#ifndef EDITFILTERLISTVIEW_H
#define EDITFILTERLISTVIEW_H

#include <QListView>
#include "editfilterwidget.h"

class QStandardItemModel;
class EditFilterItemDelegate;
class QAction;
class RosterBaseItem;

class EditFilterListView : public QListView
{
	Q_OBJECT

public:
	enum ListType
	{
		Roster,
		Os,
		Group,
		Discuss,
		Subscription
	};

public:
	EditFilterListView(QWidget *parent);
	~EditFilterListView();

	void setSelectMode(EditFilterWidget::SelectMode mode);
	EditFilterWidget::SelectMode selectMode() const;

	void setListType(ListType listType);
	ListType listType() const;

	void setEditFilterModel(QStandardItemModel *sourceModel);
	QStandardItemModel *editFilterModel() const;
	
	void addItem(RosterBaseItem *item);
	void clearAll();

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
	void returnKeyPressed();

private:
	QStandardItemModel     *m_sourceModel;
	EditFilterItemDelegate *m_itemDelegate;
	QModelIndex             m_clickedIndex;

	ListType                m_listType;

	QAction                *m_chatAction;
	QAction                *m_groupAction;
	QAction                *m_discussAction;
	QAction                *m_subscriptionAction;
	QAction                *m_selectAction;
	QAction                *m_viewMaterialAction;

	EditFilterWidget::SelectMode m_selectMode;
};

#endif // EDITFILTERLISTVIEW_H
