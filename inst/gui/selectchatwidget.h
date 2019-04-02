#ifndef SELECTCHATWIDGET_H
#define SELECTCHATWIDGET_H

#include <QWidget>
namespace Ui {class SelectChatWidget;};

class SelectChatTreeModel;
class SelectChatListModel;
class EditFilterWidget;

class SelectChatWidget : public QWidget
{
	Q_OBJECT

public:
	enum Source
	{
		RosterSource      = 0,
		OsSource          = 1,
		GroupSource       = 2,
		DiscussSource     = 3,
		LastContactSource = 4,
		SourceInvalid =     8
	};

public:
	SelectChatWidget(QWidget *parent = 0);
	~SelectChatWidget();

	void init(const QString &searchTip, bool showLastContact = false);
	void select(const QString &id, SelectChatWidget::Source source, const QString &extra = QString());
	void selectFirst();

	EditFilterWidget *searchWidget() const;

public Q_SLOTS:
	void editFilterChanged(const QString &filterText);

Q_SIGNALS:
	void selectChanged(const QString &id, int source);
	void itemClicked(const QString &id, int source);

private Q_SLOTS:
	void editFilterSelected(const QString &id, int source, const QString &wid);
	void onTreeViewGroupsClicked(const QModelIndex &index);
	void onListViewMembersActivated(const QModelIndex &index);
	void onListViewMembersClicked(const QModelIndex &index);

private:
	Ui::SelectChatWidget *ui;

	SelectChatTreeModel  *m_selectTreeModel;
	SelectChatListModel  *m_selectListModel;
};

#endif // SELECTCHATWIDGET_H
