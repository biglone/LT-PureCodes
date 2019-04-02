#ifndef SHOWREDUNDANTDISCUSSMEMBERDIALOG_H
#define SHOWREDUNDANTDISCUSSMEMBERDIALOG_H

#include <QDialog>
#include <QItemDelegate>

#include "framelessdialog.h"
#include "ui_showredundantdiscussmemberdialog.h"

class ShowRedundantDiscussMemberDialog : public FramelessDialog
{
	Q_OBJECT

public:
	explicit ShowRedundantDiscussMemberDialog(QStringList &redundantIdList, QWidget *parent = 0);
	~ShowRedundantDiscussMemberDialog();
	
public:
	void setSkin();

public slots:
	void on_btnOK_clicked();
	void on_btnCancel_clicked();

private:
	Ui::ShowRedundantDiscussMemberDialog ui;
};

//####SimpleDiscussItemDelegate class####
class SimpleDiscussItemDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	SimpleDiscussItemDelegate(QObject *parent);
	~SimpleDiscussItemDelegate();

public:
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // SHOWREDUNDANTDISCUSSMEMBERDIALOG_H
