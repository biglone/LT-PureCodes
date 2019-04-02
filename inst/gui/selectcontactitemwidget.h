#ifndef SELECTCONTACTITEMWIDGET_H
#define SELECTCONTACTITEMWIDGET_H

#include <QWidget>
#include <QCheckBox>

namespace Ui {class SelectContactItemWidget;};
class QStandardItem;
class QPixmap;

//////////////////////////////////////////////////////////////////////////
// class SelectContactCheckBox
class SelectContactCheckBox : public QCheckBox
{
	Q_OBJECT
public:
	SelectContactCheckBox(QWidget *parent = 0) : QCheckBox(parent) {}

protected:
	virtual void nextCheckState()
	{
		if (checkState() == Qt::Unchecked)
		{
			setCheckState(Qt::Checked);
		}
		else if (checkState() == Qt::Checked)
		{
			setCheckState(Qt::Unchecked);
		}
		else
		{
			setCheckState(Qt::Unchecked);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// class SelectContactItemWidget
class SelectContactItemWidget : public QWidget
{
	Q_OBJECT

public:
	SelectContactItemWidget(QStandardItem *itemData, bool group, const QString &name, bool enableCheck = true, QWidget *parent = 0);
	~SelectContactItemWidget();

	void setGroupExpanded(bool expanded);
	
	void setChecked(bool checked, bool partial = false); // this will not trigger the signal: itemToggled

	void setCheckEnabled(bool enabled);

	void startLoading();
	void stopLoading();

	void setTitle(const QString &title);

signals:
	void itemToggled(QStandardItem *item, bool checked);

private slots:
	void onCheckStateChanged(int state);

private:
	static void initGroupPixmap();
	static bool isIconInited();
	static void setIconInited(bool inited);

private:
	Ui::SelectContactItemWidget *ui;
	QStandardItem               *m_itemData;
	static bool                  s_inited;
	static QPixmap              *s_groupExpand;
	static QPixmap              *s_groupCollapse;
};

#endif // SELECTCONTACTITEMWIDGET_H
