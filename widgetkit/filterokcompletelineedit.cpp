#include "FilterOkCompleteLineEdit.h"  
#include <QKeyEvent>  
#include <QListView>  
#include <QAbstractItemModel>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS FilterOkCompleteLineEdit
FilterOkCompleteLineEdit::FilterOkCompleteLineEdit(QWidget *parent)  
	: FilterOkLineEdit(parent), m_model(0)
{  
	m_listView = new QListView(this);
	m_listView->setWindowFlags(Qt::ToolTip);

	m_proxyModel = new SearchItemFilterProxyModel(this);

	bool connected = false;
	connected = connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(setCompleter(const QString &)));  
	Q_ASSERT(connected);

	connected = connect(m_listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(completeText(const QModelIndex &)));  
	Q_ASSERT(connected);

	connected = connect(m_listView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(completeText(const QModelIndex &)));
	Q_ASSERT(connected);
}

QListView * FilterOkCompleteLineEdit::view() const
{
	return m_listView;
}

void FilterOkCompleteLineEdit::setModel(QAbstractItemModel *model)
{
	if (m_model)
	{
		delete m_model;
		m_model = 0;
	}

	m_model = model;
	m_proxyModel->setSourceModel(m_model);
	m_listView->setModel(m_proxyModel);
}

QAbstractItemModel* FilterOkCompleteLineEdit::model() const
{
	return m_model;
}

void FilterOkCompleteLineEdit::focusOutEvent(QFocusEvent *e) 
{
	FilterOkLineEdit::focusOutEvent(e);
}

void FilterOkCompleteLineEdit::keyPressEvent(QKeyEvent *e) 
{  
	if (!m_listView->isHidden()) 
	{  
		int key = e->key();  
		int count = m_listView->model()->rowCount();  
		QModelIndex currentIndex = m_listView->currentIndex();  
		if (Qt::Key_Down == key) 
		{
			int row = currentIndex.row() + 1;  
			if (row >= count) {  
				row = 0;  
			}  
			QModelIndex index = m_listView->model()->index(row, 0);  
			m_listView->setCurrentIndex(index);
		} 
		else if (Qt::Key_Up == key) 
		{  
			int row = currentIndex.row() - 1;  
			if (row < 0) {  
				row = count - 1;  
			}  
			QModelIndex index = m_listView->model()->index(row, 0);  
			m_listView->setCurrentIndex(index);  
		} 
		else if (Qt::Key_Escape == key) 
		{  
			m_listView->hide();  
		} 
		else if (Qt::Key_Enter == key || Qt::Key_Return == key) 
		{  
			if (currentIndex.isValid()) {  
				QString text = m_listView->currentIndex().data().toString();  
				setText(text);  
			}  
			m_listView->hide();

			QLineEdit::keyPressEvent(e);
		} else {  
			m_listView->hide();  
			QLineEdit::keyPressEvent(e);  
		}  
	} 
	else 
	{  
		QLineEdit::keyPressEvent(e);  
	}  
}  

void FilterOkCompleteLineEdit::setCompleter(const QString &text) 
{  
	if (text.isEmpty()) 
	{  
		m_listView->hide();  
		return;  
	}

	m_proxyModel->setFilterRegExp(text);
	m_proxyModel->invalidate();

	if (m_proxyModel->rowCount() == 0) 
	{  
		if (!m_listView->isHidden())
			m_listView->hide();
		return;  
	}  

	// Position the text edit  
	if (m_listView->isHidden())
	{
		m_listView->setMinimumWidth(width());  
		m_listView->setMaximumWidth(width());  
		QPoint p(0, height());  
		int x = mapToGlobal(p).x();  
		int y = mapToGlobal(p).y() + 1;  
		m_listView->move(x, y);  
		m_listView->show();
	}
}  

void FilterOkCompleteLineEdit::completeText(const QModelIndex &index) 
{  
	if (index.isValid())
	{
		QString text = index.data().toString();  
		setText(text);

		emit rightButtonClicked();
	}
	m_listView->hide();
}

void FilterOkCompleteLineEdit::updateViewPosition()
{
	if (m_listView->isVisible())
	{
		QPoint p(0, height());  
		int x = mapToGlobal(p).x();  
		int y = mapToGlobal(p).y() + 1;  
		m_listView->move(x, y);
	}
}

void FilterOkCompleteLineEdit::updateHide()
{
	if (m_listView->isVisible())
		m_listView->hide();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SearchItemFilterProxyModel
SearchItemFilterProxyModel::SearchItemFilterProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{

}

bool SearchItemFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	QString patternString = filterRegExp().pattern();
	return sourceModel()->data(index).toString().contains(patternString, Qt::CaseInsensitive);
}
