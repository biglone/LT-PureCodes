#ifndef FILTEROKCOMPLETELINEEDIT_H
#define FILTEROKCOMPLETELINEEDIT_H

#include "widgetkit_global.h"
#include "FilterOkLineEdit.h"
#include <QSortFilterProxyModel>

class QListView;
class QModelIndex;
class QAbstractItemModel;

class WIDGETKIT_EXPORT FilterOkCompleteLineEdit : public FilterOkLineEdit 
{  
	Q_OBJECT  

public:  
	FilterOkCompleteLineEdit(QWidget *parent = 0);

	QListView *view() const;

	void setModel(QAbstractItemModel *model);
	QAbstractItemModel* model() const;

public slots:  
	void setCompleter(const QString &text);
	void completeText(const QModelIndex &index);
	void updateViewPosition();
	void updateHide();

protected:  
	virtual void keyPressEvent(QKeyEvent *e);  
	virtual void focusOutEvent(QFocusEvent *e);  

private:  
	QListView             *m_listView;
	QAbstractItemModel    *m_model;
	QSortFilterProxyModel *m_proxyModel;
};  

class SearchItemFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	SearchItemFilterProxyModel(QObject *parent);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // FILTEROKCOMPLETELINEEDIT_H
