#ifndef GROUPPANEL_H
#define GROUPPANEL_H

#include <QWidget>
namespace Ui {class GroupPanel;};

class GroupListView;
class DiscussTreeView;

class GroupPanel : public QWidget
{
	Q_OBJECT

public:
	GroupPanel(GroupListView *groupListView, DiscussTreeView *discussTreeView, QWidget *parent = 0);
	~GroupPanel();

signals:
	void createDiscuss();

public slots:
	void activeGroupPage();
	void activeDiscussPage();

private slots:
	void currentPageChanged(int index);

private:
	Ui::GroupPanel *ui;
};

#endif // GROUPPANEL_H
