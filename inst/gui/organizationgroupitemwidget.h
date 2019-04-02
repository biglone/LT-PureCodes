#ifndef ORGANIZATIONGROUPITEMWIDGET_H
#define ORGANIZATIONGROUPITEMWIDGET_H

#include <QWidget>
namespace Ui {class OrganizationGroupItemWidget;};

class OrganizationGroupItemWidget : public QWidget
{
	Q_OBJECT

public:
	OrganizationGroupItemWidget(QWidget *parent = 0);
	~OrganizationGroupItemWidget();

	void setGroupExpanded(bool expanded);
	void setGroupName(const QString &name);
	void startLoading();
	void stopLoading();

private:
	static void initGroupPixmap();
	static bool isIconInited();
	static void setIconInited(bool inited);

private:
	Ui::OrganizationGroupItemWidget *ui;

	static bool     s_inited;
	static QPixmap *s_groupExpand;
	static QPixmap *s_groupCollapse;
};

#endif // ORGANIZATIONGROUPITEMWIDGET_H
