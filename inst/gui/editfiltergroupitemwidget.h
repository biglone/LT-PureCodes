#ifndef EDITFILTERGROUPITEMWIDGET_H
#define EDITFILTERGROUPITEMWIDGET_H

#include <QWidget>
namespace Ui {class EditFilterGroupItemWidget;};

class EditFilterGroupItemWidget : public QWidget
{
	Q_OBJECT

public:
	enum ItemType
	{
		Roster = 0,
		Os,
		Group,
		Discuss,
		Subscription
	};

public:
	EditFilterGroupItemWidget(QWidget *parent = 0);
	~EditFilterGroupItemWidget();

	void setType(ItemType itemType);
	void setGroupText(const QString &groupText);
	void setViewAllVisible(bool visible);

signals:
	void viewAll(int type);

private slots:
	void on_viewAllLabel_clicked();

private:
	Ui::EditFilterGroupItemWidget *ui;

	ItemType m_type;
};

#endif // EDITFILTERGROUPITEMWIDGET_H
