#include "selectcontactwidget.h"
#include "ui_selectcontactwidget.h"
#include "PmApp.h"
#include "ModelManager.h"
#include <QPainter>
#include <QScrollBar>
#include "model/rostermodeldef.h"
#include "model/orgstructmodeldef.h"
#include "model/orgstructitemdef.h"
#include "util/PinYinConverter.h"
#include <QFile>
#include <QBitmap>
#include "styletoolbutton.h"
#include <QDebug>
#include "editfiltertreeview.h"
#include "editfilterlistview.h"
#include <QMouseEvent>

static const int TypeRole       = Qt::UserRole + 1024;
static const int IDRole         = TypeRole + 1;
static const int IndexRole      = TypeRole + 2;
static const int ShowRole       = TypeRole + 3;
static const int GroupIndexRole = TypeRole + 4;
static const int GroupNameRole  = TypeRole + 5;
static const int ExpandRole     = TypeRole + 6;
static const int CheckEnableRole= TypeRole + 7;
static const int LoadingRole    = TypeRole + 8;

enum TypeRoleType
{
	TypeRoleGroup,
	TypeRoleContact
};

static bool osSourceLessThan(RosterBaseItem *left, RosterBaseItem *right)
{
	if (left->itemType() != right->itemType())
	{
		if (left->itemType() == RosterBaseItem::RosterTypeGroup)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return left->itemIndex() < right->itemIndex();
	}
}

//////////////////////////////////////////////////////////////////////////
// class SelectContactGroupItem
class SelectContactGroupItem : public QStandardItem
{
public:
	SelectContactGroupItem(const QString &name, bool enableCheck = true);
	~SelectContactGroupItem();

	void setGroupExpanded(bool expanded);
	bool groupExpanded() const;

	void setCheckEnabled(bool enabled);
	bool checkEnabled() const;

	void startLoading();
	void stopLoading();
	bool isLoading() const;

private:
	static bool                  s_inited;
	static QPixmap              *s_groupExpand;
	static QPixmap              *s_groupCollapse;
};

SelectContactGroupItem::SelectContactGroupItem(const QString &name, bool enableCheck /*= true*/)
	: QStandardItem(name)
{
	setGroupExpanded(false);
	stopLoading();
	setCheckEnabled(enableCheck);
}

SelectContactGroupItem::~SelectContactGroupItem()
{

}

void SelectContactGroupItem::setGroupExpanded(bool expanded)
{
	setData(expanded, ExpandRole);
}

bool SelectContactGroupItem::groupExpanded() const
{
	return data(ExpandRole).toBool();
}

void SelectContactGroupItem::setCheckEnabled(bool enabled)
{
	setData(enabled, CheckEnableRole);
}

bool SelectContactGroupItem::checkEnabled() const
{
	return data(CheckEnableRole).toBool();
}

void SelectContactGroupItem::startLoading()
{
	setData(true, LoadingRole);
}

void SelectContactGroupItem::stopLoading()
{
	setData(false, LoadingRole);
}

bool SelectContactGroupItem::isLoading() const
{
	return data(LoadingRole).toBool();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SelectContactWidget
SelectContactWidget::SelectContactWidget(QWidget *parent /*= 0*/)
	: QWidget(parent), m_selectSource(SelectAll), m_includeStrangerGroup(true), m_maxSelect(0)
{
	ui = new Ui::SelectContactWidget();
	ui->setupUi(this);

	setSkin();
}

SelectContactWidget::~SelectContactWidget()
{
	delete ui;
}

void SelectContactWidget::init(SelectSource source /*= SelectAll*/, 
							   const QStringList &rosterFilterIds /*= QStringList()*/, 
							   const QStringList &osFilterIds /*= QStringList()*/, 
							   const QStringList &preAddIds /*= QStringList()*/,
							   const QStringList &fixedIds /*= QStringList()*/,
							   int maxSelect /*= 0*/)
{
	m_rosterFilterIds = rosterFilterIds;
	m_osFilterIds = osFilterIds;
	m_preAddIds = preAddIds;
	m_fixedIds = fixedIds;
	m_maxSelect = maxSelect;

	// construct roster source proxy model
	m_sourceRosterModel = new QStandardItemModel(this);

	// construct roster source proxy model
	m_sourceOsModel = new QStandardItemModel(this);

	// construct dest proxy model
	m_destModel = new QStandardItemModel(this);

	initUI();
	initRosterSourceModel();
	initOSSourceModel();
	initDestModel();
	onSelectionChanged();

	setSelectSource(source);
	ui->treeViewRosterSource->expandAll();

	connect(ui->pushButtonRoster, SIGNAL(clicked()), this, SLOT(sourceViewChanged()));
	connect(ui->pushButtonOs, SIGNAL(clicked()), this, SLOT(sourceViewChanged()));
	connect(ui->tBtnRemoveAll, SIGNAL(clicked()), this, SLOT(removeAll()));
	/*
	connect(ui->labelSelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
	connect(ui->labelDeselectAll, SIGNAL(clicked()), this, SLOT(deselectAll()));
	connect(ui->labelRemove, SIGNAL(clicked()), this, SLOT(remove()));
	connect(ui->labelRemoveAll, SIGNAL(clicked()), this, SLOT(removeAll()));
	*/
	connect(ui->listViewDest, SIGNAL(clicked(const QModelIndex&)), this, SLOT(destClicked(const QModelIndex&)));
	connect(ui->treeViewRosterSource, SIGNAL(clicked(QModelIndex)), this, SLOT(rosterSourceClicked(QModelIndex)));
	// connect(ui->treeViewOSSource, SIGNAL(clicked(QModelIndex)), this, SLOT(osSourceClicked(QModelIndex)));
	connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
	// filter line edit signal & slot
	connect(ui->leditFilter, SIGNAL(filterChanged(QString)), this, SLOT(editFilterChanged(QString)));
	connect(ui->leditFilter, SIGNAL(gainFocus()), this, SLOT(editFilterGainFocus()));
	connect(ui->searchPage, SIGNAL(selectSearchItem(QString, int, QString)), this, SLOT(editFilterSelected(QString, int, QString)));

	QScrollBar *vScrollBar = ui->treeViewRosterSource->verticalScrollBar();
	if (vScrollBar)
	{
		connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(rosterTreeViewScrolled()));
		connect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(rosterTreeViewScrolled()));
	}

	vScrollBar = ui->treeViewOSSource->verticalScrollBar();
	if (vScrollBar)
	{
		connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(osTreeViewScrolled()));
		connect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(osTreeViewScrolled()));
	}
}

QStringList SelectContactWidget::selectionIds() const
{
	return m_destContactItems.keys();
}

void SelectContactWidget::setSelectSource(SelectSource source)
{
	m_selectSource = source;
	if (m_selectSource == SelectAll)
	{
		ui->pushButtonRoster->setChecked(true);
		ui->pushButtonOs->setChecked(false);
		ui->stackedWidgetSource->setCurrentIndex(0);
	}
	else if (m_selectSource == SelectRoster)
	{
		ui->pushButtonRoster->setChecked(true);
		ui->pushButtonOs->setChecked(false);
		ui->pushButtonOs->setVisible(false);
		ui->stackedWidgetSource->setCurrentIndex(0);
	}
	else if (m_selectSource == SelectOS)
	{
		ui->pushButtonRoster->setChecked(false);
		ui->pushButtonOs->setChecked(true);
		ui->pushButtonRoster->setVisible(false);
		ui->stackedWidgetSource->setCurrentIndex(1);
	}
}

SelectContactWidget::SelectSource SelectContactWidget::getSelectSource() const
{
	return m_selectSource;
}

int SelectContactWidget::maxSelect() const
{
	return m_maxSelect;
}

void SelectContactWidget::setMaxSelect(int max)
{
	m_maxSelect = max;
}

void SelectContactWidget::clearDest()
{
	m_destModel->clear();
	m_destContactItems.clear();
}

void SelectContactWidget::setHeaderVisible(SelectContactSourceHV *headerView, bool visible)
{
	if (!headerView)
	{
		return;
	}

	QScrollBar *vScrollBar = 0;
	QTreeView *treeView = 0;
	if (headerView == m_rosterHeaderView)
	{
		treeView = ui->treeViewRosterSource;
	}
	else if (headerView == m_osHeaderView)
	{
		treeView = ui->treeViewOSSource;
	}
	if (!treeView)
	{
		return;
	}

	vScrollBar = treeView->verticalScrollBar();
	if (!vScrollBar)
	{
		treeView->setHeaderHidden(!visible);
		return;
	}

	if (visible)
	{
		if (treeView->isHeaderHidden())
		{
			if (treeView == ui->treeViewRosterSource)
			{
				disconnect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(rosterTreeViewScrolled()));
				disconnect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(rosterTreeViewScrolled()));
			}
			else
			{
				disconnect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(osTreeViewScrolled()));
				disconnect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(osTreeViewScrolled()));
			}

			treeView->setHeaderHidden(false);

			if (treeView == ui->treeViewRosterSource)
			{
				connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(rosterTreeViewScrolled()));
				connect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(rosterTreeViewScrolled()));
			}
			else
			{
				connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(osTreeViewScrolled()));
				connect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(osTreeViewScrolled()));
			}
		}
	}
	else
	{
		if (!treeView->isHeaderHidden())
		{
			if (treeView == ui->treeViewRosterSource)
			{
				disconnect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(rosterTreeViewScrolled()));
				disconnect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(rosterTreeViewScrolled()));
			}
			else
			{
				disconnect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(osTreeViewScrolled()));
				disconnect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(osTreeViewScrolled()));
			}

			treeView->setHeaderHidden(true);
			
			if (treeView == ui->treeViewRosterSource)
			{
				connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(rosterTreeViewScrolled()));
				connect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(rosterTreeViewScrolled()));
			}
			else
			{
				connect(vScrollBar, SIGNAL(valueChanged(int)), this, SLOT(osTreeViewScrolled()));
				connect(vScrollBar, SIGNAL(rangeChanged(int, int)), this, SLOT(osTreeViewScrolled()));
			}
		}
	}
}

QStringList SelectContactWidget::getRosterIds()
{
	return m_rosterContactItems.keys();
}

QStringList SelectContactWidget::getOsWids()
{
	return m_osContactItems.keys();
}

void SelectContactWidget::setSkin()
{
	/*
	ui->labelSelectAll->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelDeselectAll->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelRemove->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelRemoveAll->setFontAtt(QColor(0, 120, 216), 10, false);
	*/

	ui->leditFilter->setStyleSheet(
	"QLineEdit#leditFilter {"
	"	background: rgb(223, 223, 223);"
	"	border: none;"
	"	border-image:none;"
	"}"
	"QLineEdit#leditFilter:focus {"
	"	background: rgb(255, 255, 255);"
	"}");

	ui->widgetDestAction->setStyleSheet(QString("QWidget#widgetDestAction {border-top: 1px solid rgb(219, 219, 219);}"));

	ui->leftWidget->setStyleSheet(QString("QWidget#leftWidget {"
		"border: 1px solid rgb(219, 219, 219);"
		"background: rgb(240, 240, 240);"
		"}"));

	ui->rightWidget->setStyleSheet(QString("QWidget#rightWidget {"
		"border: 1px solid rgb(219, 219, 219);"
		"background: rgb(240, 240, 240);"
		"}"));

	ui->widgetSelectSource->setStyleSheet(
		"QWidget#widgetSelectSource {"
		"background-color: rgb(232, 232, 232);"
		"}"

		"QPushButton {"
		"padding-left: 0px;"
		"padding-right: 0px;"
		"border: none;"
		"border-bottom: 1px solid rgb(219, 219, 219);"
		"border-image: none;"
		"background-color: rgb(232, 232, 232);"
		"font-size: 11pt;"
		"color: black;"
		"}"
		"QPushButton:!checked:hover:!pressed {"
		"border-image: none;"
		"color: rgb(0, 120, 206);"
		"}"
		"QPushButton:checked {"
		"border-image: none;"
		"background-color: rgb(240, 240, 240);"
		"color: black;"
		"border: none;"
		"border-top: 3px solid rgb(0, 120, 216);"
		"border-left: 1px solid rgb(219, 219, 219);"
		"border-right: 1px solid rgb(219, 219, 219);"
		"}"

		"QLabel#labelSpace, QLabel#labelSpaceLeft {"
		"border: none;"
		"border-bottom: 1px solid rgb(219, 219, 219);"
		"}"
		);

	QString listQss = QString("QListView {"
		"border: none;"
		"background: rgb(240, 240, 240);"
		"}"

		"QListView {"
		"alternate-background-color: rgb(232, 232, 232);"
		"}");

	QString treeQss = QString("QTreeView {"
		"border: none;"
		"background: rgb(240, 240, 240);"
		"}"

		"QTreeView::branch:has-siblings:!adjoins-item {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:has-siblings:adjoins-item {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:has-children:!has-siblings:closed,"
		"QTreeView::branch:closed:has-children:has-siblings {"
		"border-image: none;"
		"image: none;"
		"}"

		"QTreeView::branch:open:has-children:!has-siblings,"
		"QTreeView::branch:open:has-children:has-siblings  {"
		"border-image: none;"
		"image: none"
		"}"
		);

	ui->listViewDest->setStyleSheet(listQss);
	ui->treeViewRosterSource->setStyleSheet(treeQss);
	ui->treeViewOSSource->setStyleSheet(treeQss);
}

void SelectContactWidget::sourceViewChanged()
{
	if (sender() == ui->pushButtonRoster)
	{
		ui->pushButtonRoster->setChecked(true);
		ui->pushButtonOs->setChecked(false);
	}
	else
	{
		ui->pushButtonOs->setChecked(true);
		ui->pushButtonRoster->setChecked(false);
	}

	if (ui->pushButtonRoster->isChecked())
	{
		ui->stackedWidgetSource->setCurrentIndex(0);
	}
	else
	{
		ui->stackedWidgetSource->setCurrentIndex(1);
	}
}

void SelectContactWidget::selectAll()
{
	/*
	foreach (QStandardItem *destItem, m_destContactItems.values())
	{
		if (destItem->isEnabled())
			destItem->setCheckState(Qt::Checked);
	}
	ui->listViewDest->update();
	*/
}

void SelectContactWidget::deselectAll()
{
	/*
	foreach (QStandardItem *destItem, m_destContactItems.values())
	{
		if (destItem->isEnabled())
			destItem->setCheckState(Qt::Unchecked);
	}
	ui->listViewDest->update();
	*/
}

void SelectContactWidget::remove()
{
	/*
	for (int i = m_destModel->rowCount()-1; i >= 0; i--)
	{
		QStandardItem *destItem = m_destModel->item(i);
		if (destItem->isEnabled() && destItem->checkState() == Qt::Checked)
		{
			doRemove(destItem);
		}
	}

	emit selectionChanged();
	*/
}

void SelectContactWidget::removeAll()
{
	if (m_destContactItems.isEmpty())
		return;

	// clear dest model
	m_destModel->clear();
	m_destContactItems.clear();

	// remove from dest source item
	bool hasRosterSelected = false;
	bool hasOsSelected = false;
	foreach (QString destSourceId, m_destSourceItems.keys())
	{
		QStandardItem *selItem = m_destSourceItems[destSourceId];
		SelectSource selectSource = selectSourceFromDestSourceId(destSourceId);
		if (selectSource == SelectRoster) // is roster
		{
			hasRosterSelected = true;

			selItem->setCheckState(Qt::Unchecked);
		}
		else if (selectSource == SelectOS) // is os contact
		{
			hasOsSelected = true;

			selItem->setCheckState(Qt::Unchecked);
		}
	}
	m_destSourceItems.clear();

	if (hasRosterSelected)
	{
		// de-select all roster groups
		foreach (QStandardItem *selItem, m_rosterGroupItems)
		{
			selItem->setCheckState(Qt::Unchecked);
		}
	}

	if (hasOsSelected)
	{
		// de-select all os groups
		foreach (QStandardItem *selItem, m_osGroupItems)
		{
			selItem->setCheckState(Qt::Unchecked);
		}
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	foreach (QString id, m_fixedIds)
	{
		QString name = modelManager->userName(id);
		QStandardItem *destItem = new QStandardItem(name);
		destItem->setData(TypeRoleContact, TypeRole);
		destItem->setData(id, IDRole);
		destItem->setEnabled(false);
		m_destModel->appendRow(destItem);
		m_destContactItems[id] = destItem;

		// add roster item
		if (m_rosterContactItems.contains(id))
		{
			QStandardItem *selItem = m_rosterContactItems[id];
			doRosterAdd(selItem);
		}

		// add os items
		QStringList fixedWids = uid2Wid(id);
		foreach (QString wid, fixedWids)
		{
			if (m_osContactItems.contains(wid))
			{
				QStandardItem *selItem = m_osContactItems[wid];
				doOsAdd(selItem);
			}
		}
	}

	emit selectionChanged();
}

void SelectContactWidget::remove(const QString &id)
{
	if (id.isEmpty())
		return;

	if (!m_destContactItems.contains(id))
		return;

	QStandardItem *destItem = m_destContactItems[id];
	doRemove(destItem);

	emit selectionChanged();
}

void SelectContactWidget::onFilterChanged(const QString &newFilter, const QString &oldFilter)
{
	Q_UNUSED(newFilter);
	Q_UNUSED(oldFilter);
}

void SelectContactWidget::onFilterGainFocus()
{
	if (ui->stackedWidgetSource->currentIndex() == 0)
	{
		ui->treeViewRosterSource->clearSelection();
	}
	else
	{
		ui->treeViewOSSource->clearSelection();
	}
}

void SelectContactWidget::destClicked(const QModelIndex &index)
{
	Q_UNUSED(index);

	/*
	if (!index.isValid())
		return;

	QModelIndex sourceIndex = index;
	QStandardItem *selItem = m_destModel->itemFromIndex(sourceIndex);
	if (!selItem)
		return;

	if (!selItem->isEnabled())
		return;

	if (selItem->checkState() == Qt::Checked)
		selItem->setCheckState(Qt::Unchecked);
	else
		selItem->setCheckState(Qt::Checked);
	ui->listViewDest->update();
	*/
}

void SelectContactWidget::rosterSourceClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QModelIndex sourceIndex = index;
	QStandardItem *selItem = m_sourceRosterModel->itemFromIndex(sourceIndex);
	if (!selItem)
		return;

	if (selItem->data(TypeRole).toInt() == (int)TypeRoleGroup)
	{
		// can't expand or collapse
	}
	else
	{
		if (selItem->isEnabled())
		{
			if (selItem->checkState() == Qt::Unchecked)
			{
				selItem->setCheckState(Qt::Checked);
				onItemToggled(selItem, true);
			}
			else if (selItem->checkState() == Qt::Checked)
			{
				selItem->setCheckState(Qt::Unchecked);
				onItemToggled(selItem, false);
			}
		}
	}
}

void SelectContactWidget::osSourceClicked(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QModelIndex sourceIndex = index;
	QStandardItem *selItem = m_sourceOsModel->itemFromIndex(sourceIndex);
	if (!selItem)
		return;

	if (!selItem->isEnabled())
		return;

	if (selItem->data(TypeRole).toInt() == (int)TypeRoleGroup)
	{
		SelectContactGroupItem *selGroupItem = static_cast<SelectContactGroupItem *>(selItem);
		if (ui->treeViewOSSource->isExpanded(index))
		{
			ui->treeViewOSSource->collapse(index);
			selGroupItem->setGroupExpanded(false);
			selGroupItem->stopLoading();
		}
		else
		{
			ui->treeViewOSSource->expand(index);
			selGroupItem->setGroupExpanded(true);
			selGroupItem->setEnabled(false);
			selGroupItem->startLoading();
			selGroupItem->setCheckEnabled(false);
			if (!checkToAddOsContact(selGroupItem->data(IDRole).toString()))
			{
				selGroupItem->setEnabled(true);
				selGroupItem->stopLoading();
				selGroupItem->setCheckEnabled(true);
			}
		}
	}
	else 
	{
		if (selItem->isEnabled())
		{
			if (selItem->checkState() == Qt::Unchecked)
			{
				selItem->setCheckState(Qt::Checked);
				onItemToggled(selItem, true);
			}
			else if (selItem->checkState() == Qt::Checked)
			{
				selItem->setCheckState(Qt::Unchecked);
				onItemToggled(selItem, false);
			}
		}
	}
}

void SelectContactWidget::rosterTreeViewScrolled()
{
	QPoint topPt(30, 15);
	QModelIndex topIndex = ui->treeViewRosterSource->indexAt(topPt);
	if (!topIndex.isValid())
		return;

	QModelIndex sourceIndex = topIndex;
	QStandardItem *item = m_sourceRosterModel->itemFromIndex(sourceIndex);
	if (!item)
		return;

	if (item->data(TypeRole).toInt() == TypeRoleContact)
	{
		QString id = item->data(IDRole).toString();
		QStandardItem *groupItem = item->parent();
		if (groupItem)
		{
			m_rosterHeaderView->setGroupItem(groupItem);
			m_rosterHeaderView->update();
			if (ui->treeViewRosterSource->isHeaderHidden())
			{
				setHeaderVisible(m_rosterHeaderView, true);
			}
		}
	}
	else if (item->data(TypeRole).toInt() == TypeRoleGroup)
	{
		m_rosterHeaderView->setGroupItem(0);
		if (!ui->treeViewRosterSource->isHeaderHidden())
		{
			setHeaderVisible(m_rosterHeaderView, false);
		}
	}
}

void SelectContactWidget::osTreeViewScrolled()
{
	QPoint topPt(30, 15);
	QModelIndex topIndex = ui->treeViewOSSource->indexAt(topPt);
	if (!topIndex.isValid())
		return;

	QModelIndex sourceIndex = topIndex;
	QStandardItem *item = m_sourceOsModel->itemFromIndex(sourceIndex);
	if (!item)
		return;

	if (item->data(TypeRole).toInt() == TypeRoleContact)
	{
		QString id = item->data(IDRole).toString();
		QStandardItem *groupItem = item->parent();
		if (groupItem)
		{
			m_osHeaderView->setGroupItem(groupItem);
			m_osHeaderView->update();
			if (ui->treeViewOSSource->isHeaderHidden())
			{
				setHeaderVisible(m_osHeaderView, true);
			}
		}
	}
	else if (item->data(TypeRole).toInt() == TypeRoleGroup)
	{
		m_osHeaderView->setGroupItem(0);
		if (!ui->treeViewOSSource->isHeaderHidden())
		{
			setHeaderVisible(m_osHeaderView, false);
		}
	}
}

void SelectContactWidget::onItemToggled(QStandardItem *item, bool checked)
{
	if (ui->stackedWidgetSource->currentIndex() == 0) // roster
	{
		if (checked)
		{
			doRosterAdd(item);
		}
		else
		{
			doRosterRemove(item);
		}
	}
	else // os
	{
		if (checked)
		{
			if (item->data(TypeRole).toInt() == TypeRoleGroup)
			{
				if (ui->treeViewOSSource->isExpanded(item->index()))
				{
					doOsAdd(item);
				}
				else
				{
					QModelIndex index = item->index();
					ui->treeViewOSSource->expand(index);

					SelectContactGroupItem *groupItem = static_cast<SelectContactGroupItem *>(item);
					groupItem->setGroupExpanded(true);
					groupItem->setCheckState(Qt::Checked);
					groupItem->setEnabled(false);
					groupItem->startLoading();
					groupItem->setCheckEnabled(false);

					if (!checkToAddOsContact(groupItem->data(IDRole).toString()))
					{
						groupItem->setEnabled(true);
						groupItem->stopLoading();
						groupItem->setCheckEnabled(true);

						doOsAdd(groupItem);
					}
				}
			}
			else
			{
				doOsAdd(item);
			}
		}
		else
		{
			doOsRemove(item);
		}
	}

	emit selectionChanged();
}

void SelectContactWidget::onSelectionChanged()
{
	// update select text
	QString selectText;
	QStringList selIds = this->selectionIds();
	int selectCount = selIds.count();
	int maxSelect = this->maxSelect();
	if (maxSelect > 0)
	{
		if (selectCount <= maxSelect)
		{
			selectText = QString("%1/%2").arg(selectCount).arg(maxSelect);
		}
		else
		{
			selectText = QString("<font color='red'>%1</font>/%2").arg(selectCount).arg(maxSelect);
		}
	}
	else
	{
		selectText = QString("%1").arg(selectCount);
	}
	ui->labelSelect->setText(selectText);
}

void SelectContactWidget::onOsChildrenAdded(const QString &groupId, const QStringList &childGroupIds)
{
	Q_UNUSED(childGroupIds);

	if (!m_osGroupItems.contains(groupId))
		return;

	QStandardItem *parentGroupItem = m_osGroupItems[groupId];
	if (parentGroupItem->isEnabled())
		return;

	// enable os group item
	parentGroupItem->setEnabled(true);
	SelectContactGroupItem *selGroupItem = static_cast<SelectContactGroupItem *>(parentGroupItem);
	selGroupItem->stopLoading();
	selGroupItem->setCheckEnabled(true);

	// add children
	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel *osModel = modelManager->orgStructModel();
	OrgStructGroupItem *osParentGroupItem = osModel->orgStructGroup(groupId);
	
	QList<RosterBaseItem *> osChildItems;
	for (int i = 0; i < osParentGroupItem->rowCount(); i++)
	{
		RosterBaseItem *item = static_cast<RosterBaseItem *>(osParentGroupItem->child(i));
		osChildItems.append(item);
	}

	qSort(osChildItems.begin(), osChildItems.end(), osSourceLessThan);

	QStandardItem *checkItem = 0;
	foreach (RosterBaseItem *item, osChildItems)
	{
		if (item->itemType() == RosterBaseItem::RosterTypeGroup) // if is child group, add to list and model
		{
			OrgStructGroupItem *osGroup = static_cast<OrgStructGroupItem *>(item);

			if (!m_osGroupItems.contains(osGroup->itemId()))
			{
				QStandardItem *groupItem = createSelectItem(false, TypeRoleGroup, osGroup->itemId(), osGroup->itemName(), osGroup->itemIndex(), -1, QString());
				m_osGroupItems[osGroup->itemId()] = groupItem;
			}

			QStandardItem *groupItem = m_osGroupItems[osGroup->itemId()];
			parentGroupItem->appendRow(groupItem);

			checkItem = groupItem;
		}
		else if (item->itemType() == RosterBaseItem::RosterTypeContact) // if is a contact, add to model
		{
			OrgStructContactItem *osContact = static_cast<OrgStructContactItem *>(item);
			QString osWid = osContact->itemWid();

			if (m_osFilterIds.contains(osModel->wid2Uid(osWid)))
			{
				continue;
			}

			if (!m_osContactItems.contains(osWid))
			{
				QStandardItem *contactItem = createSelectItem(false, TypeRoleContact, osWid, osContact->itemName(), osContact->itemIndex(), 
					osParentGroupItem->itemIndex(), osParentGroupItem->itemName());
				m_osContactItems[osWid] = contactItem;
			}

			QStandardItem *contactItem = m_osContactItems[osWid];
			parentGroupItem->appendRow(contactItem);

			checkItem = contactItem;

			if (m_fixedIds.contains(wid2Uid(osWid)))
			{
				contactItem->setEnabled(false);
			}

			QString id = wid2Uid(osWid);
			if ((parentGroupItem->checkState() != Qt::Unchecked) || 
				(m_destContactItems.contains(id)))
			{
				contactItem->setCheckState(Qt::Checked);

				// add to dest model
				addDestItem(contactItem, SelectOS);
			}
		}
	}

	// check to select group
	if (checkItem)
	{
		checkOsGroupChecked(checkItem);
	}
}

void SelectContactWidget::editFilterChanged(const QString &filterText)
{
	QString searchString = filterText;
	if (searchString.isEmpty())
	{
		if (ui->leftStackedWidget->currentIndex() != 0)
			ui->leftStackedWidget->setCurrentIndex(0);
	}
	else
	{
		if (ui->leftStackedWidget->currentIndex() != 1)
			ui->leftStackedWidget->setCurrentIndex(1);
	}

	ui->searchPage->editFilterChanged(filterText);
}

void SelectContactWidget::editFilterGainFocus()
{
	QString searchString = ui->leditFilter->text();
	if (!searchString.isEmpty())
	{
		editFilterChanged(searchString);
	}
}

void SelectContactWidget::editFilterSelected(const QString &id, int source, const QString &wid)
{
	if (source == SELECT_SOURCE_ROSTER) // roster
	{
		if (m_rosterContactItems.contains(id))
		{
			QStandardItem *item = m_rosterContactItems[id];
			doRosterAdd(item);
		}
	}
	if (source == SELECT_SOURCE_OS) // organization structure
	{
		if (m_osContactItems.contains(wid))
		{
			QStandardItem *item = m_osContactItems[wid];
			doOsAdd(item);
		}
	}

	emit selectionChanged();
}

void SelectContactWidget::initRosterSourceModel()
{
	// construct roster source model
	ModelManager *modelManager = qPmApp->getModelManager(); 
	RosterModel *rosterModel = modelManager->rosterModel();
	QStringList groupNames = rosterModel->allGroupNames();
	int groupIndex = 0;
	foreach (QString groupName, groupNames)
	{
		// add group item and item widget
		QStandardItem *groupItem = createSelectItem(false, TypeRoleGroup, QString(), groupName, groupIndex++, -1, QString());
		m_sourceRosterModel->appendRow(groupItem);
		m_rosterGroupItems[groupName] = groupItem;

		QStringList groupChildIds = rosterModel->groupRosters(groupName);
		int rosterCount = rosterGroupChildCount(groupChildIds, m_rosterFilterIds);
		QString groupTitle = QString("%1 [%2]").arg(groupName).arg(rosterCount);

		// create roster item
		int rosterIndex = 0;
		QStringList rosterIds = rosterModel->groupRosters(groupName);
		foreach (QString rosterId, rosterIds)
		{
			if (!m_rosterFilterIds.contains(rosterId))
			{
				QString rosterName = rosterModel->rosterName(rosterId);
				QStandardItem *rosterItem = createSelectItem(false, TypeRoleContact, rosterId, rosterName, rosterIndex++, -1, tr("Friends"));
				m_rosterContactItems[rosterId] = rosterItem;
			}
		}

		// add roster item
		checkToAddRoster(groupItem->text());
	}

	QStandardItem *headerItem = new QStandardItem();
	headerItem->setIcon(QIcon(":/images/Icon_42.png"));
	headerItem->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	m_sourceRosterModel->setHorizontalHeaderItem(0, headerItem);
}

void SelectContactWidget::initOSSourceModel()
{
	// construct os source model
	QList<OrgStructGroupItem *> osGroupItems;
	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel* osModel = modelManager->orgStructModel();
	QStringList groupIds = osModel->topGroupIds();

	// add top groups
	foreach (QString groupId, groupIds)
	{
		OrgStructGroupItem *osGroup = osModel->orgStructGroup(groupId);
		osGroupItems.append(osGroup);
	}

	qSort(osGroupItems.begin(), osGroupItems.end(), osSourceLessThan);

	// add top group items and item widget
	foreach (OrgStructGroupItem *osGroup, osGroupItems)
	{
		QString groupId = osGroup->itemId();
		QStandardItem *groupItem = createSelectItem(false, TypeRoleGroup, groupId, osGroup->itemName(), osGroup->itemIndex(), -1, QString());

		m_sourceOsModel->appendRow(groupItem);
		m_osGroupItems[groupId] = groupItem;
	}

	connect(osModel, SIGNAL(orgStructChildrenAdded(QString, QStringList)), this, SLOT(onOsChildrenAdded(QString, QStringList)));

	//////////////////////////////////////////////////////////////////////////
	// create other group items and contact items
	while (!osGroupItems.isEmpty())
	{
		OrgStructGroupItem *osParentGroupItem = osGroupItems.takeFirst();
		QStandardItem *parentGroupItem = m_osGroupItems[osParentGroupItem->itemId()];
		if (!osParentGroupItem || !parentGroupItem)
		{
			continue;
		}

		int childCount = osParentGroupItem->rowCount();
		for (int i = 0; i < childCount; i++)
		{
			RosterBaseItem *item = static_cast<RosterBaseItem *>(osParentGroupItem->child(i));
			if (item->itemType() == RosterBaseItem::RosterTypeGroup) // if is child group, add to list and model
			{
				OrgStructGroupItem *osGroup = static_cast<OrgStructGroupItem *>(item);
				osGroupItems.append(osGroup);

				QStandardItem *groupItem = createSelectItem(false, TypeRoleGroup, osGroup->itemId(), osGroup->itemName(), osGroup->itemIndex(), -1, QString());
				m_osGroupItems[osGroup->itemId()] = groupItem;
			}
			else if (item->itemType() == RosterBaseItem::RosterTypeContact) // if is a contact, add to model
			{
				OrgStructContactItem *osContact = static_cast<OrgStructContactItem *>(item);
				QString osWid = osContact->itemWid();
				if (!m_osFilterIds.contains(osModel->wid2Uid(osWid)))
				{
					QStandardItem *contactItem = createSelectItem(false, TypeRoleContact, osWid, osContact->itemName(), osContact->itemIndex(), 
						osParentGroupItem->itemIndex(), osParentGroupItem->itemName());
					m_osContactItems[osWid] = contactItem;
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	QStandardItem *headerItem = new QStandardItem();
	headerItem->setIcon(QIcon(":/images/Icon_42.png"));
	headerItem->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	m_sourceOsModel->setHorizontalHeaderItem(0, headerItem);
}

void SelectContactWidget::initDestModel()
{
	// add all pre-add items
	ModelManager *modelManager = qPmApp->getModelManager();
	foreach (QString id, m_preAddIds)
	{
		QString name = modelManager->userName(id);
		QStandardItem *destItem = new QStandardItem(name);
		destItem->setData(TypeRoleContact, TypeRole);
		destItem->setData(id, IDRole);
		if (m_fixedIds.contains(id))
			destItem->setEnabled(false);
		m_destModel->appendRow(destItem);
		m_destContactItems[id] = destItem;

		// add pre-add roster item
		if (m_rosterContactItems.contains(id))
		{
			QStandardItem *selItem = m_rosterContactItems[id];
			doRosterAdd(selItem);
		}
	}

	/*
	// add pre-add os contacts
	QStringList preAddWids;
	foreach (id, m_preAddIds)
	{
		QStringList wids = uid2Wid(id);
		preAddWids.append(wids);
	}

	foreach (QString wid, preAddWids)
	{
		if (m_osContactItems.contains(wid))
		{
			QStandardItem *selItem = m_osContactItems[wid];
			doOsAdd(selItem);
		}
	}
	*/
}

void SelectContactWidget::initUI()
{
	/*
	ui->labelSelectAll->setVisible(false);
	ui->labelDeselectAll->setVisible(false);
	ui->labelRemove->setVisible(false);
	ui->labelRemoveAll->setVisible(false);
	*/

	ui->pushButtonRoster->setCheckable(true);
	ui->pushButtonOs->setCheckable(true);

	ui->pushButtonRoster->setText(tr("Friends"));
	ui->pushButtonOs->setText(tr("Corporation"));

	// set to roster page
	ui->stackedWidgetSource->setCurrentIndex(0);

	// roster source tree view
	SelectContactItemDelegate *treeItemDelegate = new SelectRosterContactItemDelegate(SelectRoster, ui->treeViewRosterSource);
	ui->treeViewRosterSource->setItemDelegate(treeItemDelegate);
	ui->treeViewRosterSource->setRootIsDecorated(false);
	ui->treeViewRosterSource->setHeaderHidden(true);
	ui->treeViewRosterSource->setAnimated(false);
	ui->treeViewRosterSource->setMouseTracking(true);
	ui->treeViewRosterSource->setAttribute(Qt::WA_Hover, true);
	ui->treeViewRosterSource->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeViewRosterSource->setItemsExpandable(true);
	ui->treeViewRosterSource->setExpandsOnDoubleClick(false);
	ui->treeViewRosterSource->setContextMenuPolicy(Qt::NoContextMenu);
	ui->treeViewRosterSource->setAcceptDrops(false);
	ui->treeViewRosterSource->setDropIndicatorShown(false);
	ui->treeViewRosterSource->setAutoExpandDelay(100);
	ui->treeViewRosterSource->setIndentation(16);
	ui->treeViewRosterSource->setSortingEnabled(false);
	ui->treeViewRosterSource->setModel(m_sourceRosterModel);
	ui->treeViewRosterSource->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui->treeViewRosterSource->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_rosterHeaderView = new SelectContactSourceHV(this, m_sourceRosterModel, ui->treeViewRosterSource);
	ui->treeViewRosterSource->setHeader(m_rosterHeaderView);
	ui->treeViewRosterSource->setHeaderHidden(true);

	// os source tree view
	treeItemDelegate = new SelectContactItemDelegate(SelectOS, ui->treeViewOSSource);
	connect(treeItemDelegate, SIGNAL(itemToggled(QStandardItem *, bool)), this, SLOT(onItemToggled(QStandardItem *, bool)));
	connect(treeItemDelegate, SIGNAL(clicked(QModelIndex)), this, SLOT(osSourceClicked(QModelIndex)));
	ui->treeViewOSSource->setItemDelegate(treeItemDelegate);
	ui->treeViewOSSource->setRootIsDecorated(false);
	ui->treeViewOSSource->setHeaderHidden(true);
	ui->treeViewOSSource->setAnimated(false);
	ui->treeViewOSSource->setMouseTracking(true);
	ui->treeViewOSSource->setAttribute(Qt::WA_Hover, true);
	ui->treeViewOSSource->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeViewOSSource->setItemsExpandable(true);
	ui->treeViewOSSource->setExpandsOnDoubleClick(false);
	ui->treeViewOSSource->setContextMenuPolicy(Qt::NoContextMenu);
	ui->treeViewOSSource->setAcceptDrops(false);
	ui->treeViewOSSource->setDropIndicatorShown(false);
	ui->treeViewOSSource->setAutoExpandDelay(100);
	ui->treeViewOSSource->setIndentation(16);
	ui->treeViewOSSource->setSortingEnabled(false);
	ui->treeViewOSSource->setModel(m_sourceOsModel);
	ui->treeViewOSSource->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui->treeViewOSSource->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	m_osHeaderView = new SelectContactSourceHV(this, m_sourceOsModel, ui->treeViewOSSource);
	ui->treeViewOSSource->setHeader(m_osHeaderView);
	ui->treeViewOSSource->setHeaderHidden(true);

	// dest list view
	SelectContactDestItemDelegate *listItemDelegate = new SelectContactDestItemDelegate(ui->listViewDest);
	ui->listViewDest->setItemDelegate(listItemDelegate);
	ui->listViewDest->setContextMenuPolicy(Qt::NoContextMenu);
	ui->listViewDest->setMouseTracking(true);
	ui->listViewDest->setAttribute(Qt::WA_Hover, true);
	ui->listViewDest->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->listViewDest->setDragEnabled(false);
	ui->listViewDest->setAcceptDrops(false);
	ui->listViewDest->setDropIndicatorShown(false);
	ui->listViewDest->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->listViewDest->setModel(m_destModel);
	// ui->listViewDest->setAlternatingRowColors(true);
	connect(listItemDelegate, SIGNAL(delButtonClicked(QString)), this, SLOT(remove(QString)));

	// filter line edit
	QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
		QLatin1String("edit-clear-locationbar-rtl") :
	QLatin1String("edit-clear-locationbar-ltr"),
		QIcon::fromTheme(QLatin1String("edit-clear"), QIcon(QLatin1String(":/images/Icon_105.png"))));
	ui->leditFilter->setButtonPixmap(FilterLineEdit::Left, icon.pixmap(16));
	ui->leditFilter->setButtonVisible(FilterLineEdit::Left, true);
	ui->leditFilter->setAutoHideButton(FilterLineEdit::Left, false);
	ui->leditFilter->setPlaceholderText(tr("Search"));
	ui->leditFilter->setKeyDelegate(ui->searchPage->treeViewSearch());

	// filter line edit add completer
	ui->searchPage->setSelectMode(EditFilterWidget::SingleClick);
	ui->searchPage->setSourceDelegate(this);
	ui->searchPage->addEditFilterCompleter(true, true, false, false, false);
	ui->searchPage->setTipText(tr("Double click to choose member"));
	ui->searchPage->setTipTextVisible(false);

	// init to first page
	ui->leftStackedWidget->setCurrentIndex(0);

	// hide context view
	ui->searchPage->treeViewSearch()->setContextMenuPolicy(Qt::NoContextMenu);
	ui->searchPage->listViewSearch()->setContextMenuPolicy(Qt::NoContextMenu);
}

void SelectContactWidget::doRosterAdd(QStandardItem *selItem)
{
	if (!selItem)
		return;

	TypeRoleType itemType = (TypeRoleType)(selItem->data(TypeRole).toInt());
	if (itemType == TypeRoleContact)
	{
		QString id = selItem->data(IDRole).toString();

		// select self
		selItem->setCheckState(Qt::Checked);

		if (m_fixedIds.contains(id))
		{
			selItem->setEnabled(false);
		}

		// check to select group
		checkRosterGroupChecked(selItem);

		// add to dest model
		addDestItem(selItem, SelectRoster);
	}
	else
	{
		// select self
		selItem->setCheckState(Qt::Checked);

		// select all its children
		selectRosterGroupChildren(selItem);

		// check to select up group
		checkRosterGroupChecked(selItem);
	}
}

void SelectContactWidget::doOsAdd(QStandardItem *selItem)
{
	if (!selItem)
		return;

	TypeRoleType itemType = (TypeRoleType)(selItem->data(TypeRole).toInt());
	if (itemType == TypeRoleContact)
	{
		// select self
		selItem->setCheckState(Qt::Checked);

		QString id = wid2Uid(selItem->data(IDRole).toString());
		if (m_fixedIds.contains(id))
		{
			selItem->setEnabled(false);
		}

		// check to select group
		checkOsGroupChecked(selItem);

		// add to dest model
		addDestItem(selItem, SelectOS);
	}
	else
	{
		// select self
		selItem->setCheckState(Qt::Checked);

		// select all its children
		selectOsGroupChildren(selItem);

		// check to select up group
		if (selItem->rowCount() > 0)
			selItem = selItem->child(0);
		checkOsGroupChecked(selItem);
	}
}

void SelectContactWidget::doRemove(QStandardItem *selItem)
{
	if (!selItem)
		return;

	QString id = selItem->data(IDRole).toString();
	if (m_fixedIds.contains(id)) // fixed ids can't be removed
		return;

	// remove from dest model
	selItem = (m_destModel->takeRow(selItem->row()))[0];
	m_destContactItems.remove(id);
	delete selItem;
	selItem = 0;

	// remove from dest source item
	QString rosterDestSourceId = makeDestSourceId(id, SelectRoster);
	if (m_destSourceItems.contains(rosterDestSourceId))
	{
		QStandardItem *item = m_destSourceItems.value(rosterDestSourceId);
		doRosterRemove(item);
	}

	QStringList wids = uid2Wid(id);
	foreach (QString wid, wids)
	{
		QString osDestSourceId = makeDestSourceId(wid, SelectOS);
		if (m_destSourceItems.contains(osDestSourceId))
		{
			QStandardItem *item = m_destSourceItems.value(osDestSourceId);
			doOsRemove(item);
		}
	}
}

void SelectContactWidget::doRosterRemove(QStandardItem *selItem)
{
	if (!selItem)
	{
		return;
	}

	if (selItem->data(TypeRole).toInt() == TypeRoleContact)
	{
		QString id = selItem->data(IDRole).toString();
		if (!m_rosterContactItems.contains(id))
		{
			return;
		}

		if (!m_fixedIds.contains(id))
		{
			selItem->setCheckState(Qt::Unchecked);

			// check group select
			checkRosterGroupChecked(selItem);

			removeDestItem(selItem, SelectRoster);
		}
	}
	else
	{
		// select self
		selItem->setCheckState(Qt::Unchecked);

		// select all its children
		selectRosterGroupChildren(selItem, false);

		// check to select up group
		checkRosterGroupChecked(selItem);
	}
}

void SelectContactWidget::doOsRemove(QStandardItem *selItem)
{
	if (!selItem)
		return;

	TypeRoleType itemType = (TypeRoleType)(selItem->data(TypeRole).toInt());
	if (itemType == TypeRoleContact)
	{
		QString id = wid2Uid(selItem->data(IDRole).toString());

		// select self
		if (!m_fixedIds.contains(id))
		{
			selItem->setCheckState(Qt::Unchecked);

			// check to select group
			checkOsGroupChecked(selItem);

			removeDestItem(selItem, SelectOS);
		}
	}
	else
	{
		// select self
		selItem->setCheckState(Qt::Unchecked);

		// select all its children
		selectOsGroupChildren(selItem, false);

		// check to select up group
		checkOsGroupChecked(selItem);
	}
}

void SelectContactWidget::addDestItem(QStandardItem *sourceItem, SelectSource selectSource)
{
	if (!sourceItem)
		return;

	// get id and name
	QString id = sourceItem->data(IDRole).toString();
	QString wid = sourceItem->data(IDRole).toString();
	QString destSourceId;
	if (selectSource == SelectOS)
	{
		id = wid2Uid(wid);
		destSourceId = makeDestSourceId(wid, selectSource);
	}
	else
	{
		destSourceId = makeDestSourceId(id, selectSource);
	}

	// add to dest source items
	if (!m_destSourceItems.contains(destSourceId))
	{
		m_destSourceItems.insert(destSourceId, sourceItem);
	}
	
	// add to dest model
	if (!m_destContactItems.contains(id))
	{
		QString name = sourceItem->text();
		QStandardItem *destItem = new QStandardItem(name);
		destItem->setData(TypeRoleContact, TypeRole);
		destItem->setData(id, IDRole);
		if (m_fixedIds.contains(id))
			destItem->setEnabled(false);
		else
			destItem->setEnabled(true);
		m_destModel->appendRow(destItem);
		m_destContactItems[id] = destItem;
		ui->listViewDest->scrollToBottom();
	}
}

void SelectContactWidget::removeDestItem(QStandardItem *sourceItem, SelectSource selectSource)
{
	if (!sourceItem)
		return;

	// get id and name
	QString id = sourceItem->data(IDRole).toString();
	QString wid = sourceItem->data(IDRole).toString();
	QString destSourceId;
	if (selectSource == SelectOS)
	{
		id = wid2Uid(wid);
		destSourceId = makeDestSourceId(wid, selectSource);
	}
	else
	{
		destSourceId = makeDestSourceId(id, selectSource);
	}

	if (m_fixedIds.contains(id)) // can't remove fixed items
		return;

	// remove from dest source item
	if (m_destSourceItems.contains(destSourceId))
	{
		m_destSourceItems.remove(destSourceId);
	}

	// remove from dest model
	QStringList checkSourceIds;
	checkSourceIds << makeDestSourceId(id, SelectRoster);
	QStringList checkWids = uid2Wid(id);
	foreach (QString checkWid, checkWids)
	{
		checkSourceIds << makeDestSourceId(checkWid, SelectOS);
	}
	QSet<QString> destSourceSet = m_destSourceItems.keys().toSet();
	QSet<QString> checkSourceSet = checkSourceIds.toSet();
	if (destSourceSet.intersect(checkSourceSet).isEmpty())
	{
		if (m_destContactItems.contains(id))
		{
			QStandardItem *destItem = m_destContactItems[id];
			m_destModel->takeRow(destItem->index().row());
			m_destContactItems.remove(id);
			delete destItem;
			destItem = 0;
		}
	}
}

void SelectContactWidget::checkToAddRoster(const QString &groupName)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	if (!rosterModel->containsGroup(groupName))
	{
		return;
	}

	if (!m_rosterGroupItems.contains(groupName))
	{
		return;
	}

	QStringList rosterIds = rosterModel->groupRosters(groupName);
	if (rosterIds.count() <= 0)
	{
		return;
	}

	QStandardItem *groupItem = m_rosterGroupItems[groupName];
	if (groupItem->rowCount() > 0) // added before
	{
		return;
	}
	
	foreach (QString rosterId, rosterIds)
	{
		if (m_rosterFilterIds.contains(rosterId))
		{
			continue;
		}

		if (!m_rosterContactItems.contains(rosterId))
		{
			continue;
		}

		QStandardItem *rosterItem = m_rosterContactItems[rosterId];
		if (m_fixedIds.contains(rosterId))
		{
			rosterItem->setEnabled(false);
		}
		groupItem->appendRow(rosterItem);
	}
}

bool SelectContactWidget::checkToAddOsContact(const QString &groupId)
{
	// construct os source model
	QStandardItem *parentGroupItem = m_osGroupItems[groupId];
	if (parentGroupItem->rowCount() > 0) // added before
	{
		return false;
	}

	// add group children from os model
	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel *osModel = modelManager->orgStructModel();
	osModel->checkToAddChildren(groupId);
	return true;
}

int SelectContactWidget::rosterGroupChildCount(const QStringList &uids, const QStringList &subIds)
{
	if (subIds.isEmpty())
	{
		return uids.count();
	}

	QSet<QString> rawSet = uids.toSet();
	QSet<QString> subSet = subIds.toSet();
	QSet<QString> leftSet = rawSet - subSet;
	return leftSet.count();
}

int SelectContactWidget::osGroupChildCount(const QStringList &wids, const QStringList &subIds)
{
	if (subIds.isEmpty())
	{
		return wids.count();
	}

	QStringList subWids;
	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel* osModel = modelManager->orgStructModel();
	foreach (QString subId, subIds)
	{
		QStringList ret = osModel->uid2Wid(subId);
		subWids.append(ret);
	}

	QSet<QString> rawSet = wids.toSet();
	QSet<QString> subSet = subWids.toSet();
	QSet<QString> leftSet = rawSet - subSet;

	return leftSet.count();
}

void SelectContactWidget::checkRosterGroupChecked(QStandardItem *childItem)
{
	if (!childItem)
	{
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	QString id = childItem->data(IDRole).toString();
	QString groupName = rosterModel->rosterGroupName(id);

	if (!m_rosterGroupItems.contains(groupName))
	{
		return;
	}

	QStringList rosterIds = rosterModel->groupRosters(groupName);
	bool hasChecked = false;
	bool hasUnchecked = false;
	foreach (QString rosterId, rosterIds)
	{
		if (m_rosterContactItems.contains(rosterId))
		{
			QStandardItem *rosterContactItem = m_rosterContactItems[rosterId];
			if (rosterContactItem->checkState() == Qt::Checked)
			{
				hasChecked = true;
			}
			else
			{
				hasUnchecked = true;
			}

			if (hasChecked && hasUnchecked)
			{
				break;
			}
		}
	}

	Qt::CheckState checkState = Qt::Checked;
	if (hasChecked && hasUnchecked)
	{
		checkState = Qt::PartiallyChecked;
	}
	else if (hasChecked)
	{
		checkState = Qt::Checked;
	}
	else
	{
		checkState = Qt::Unchecked;
	}

	QStandardItem *groupItem = m_rosterGroupItems[groupName];
	groupItem->setCheckState(checkState);
}

void SelectContactWidget::checkOsGroupChecked(QStandardItem *childItem)
{
	if (!childItem)
	{
		return;
	}

	OrgStructGroupItem *parentItem = 0;

	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel *osModel = modelManager->orgStructModel();

	if (childItem->data(TypeRole).toInt() == TypeRoleContact)
	{
		QString wid = childItem->data(IDRole).toString();
		OrgStructContactItem *contactItem = osModel->contactByWid(wid);
		if (!contactItem)
		{
			return;
		}

		parentItem = static_cast<OrgStructGroupItem *>(contactItem->parent());
	}
	else
	{
		QString groupId = childItem->data(IDRole).toString();
		OrgStructGroupItem *groupItem = osModel->orgStructGroup(groupId);
		if (!groupItem)
		{
			return;
		}

		parentItem = static_cast<OrgStructGroupItem *>(groupItem->parent());
	}

	if (!parentItem)
	{
		return;
	}

	if (!m_osGroupItems.contains(parentItem->itemId()))
	{
		return;
	}

	bool hasChecked = false;
	bool hasUnchecked = false;
	for (int i = 0; i < parentItem->rowCount(); i++)
	{
		RosterBaseItem *osChildItem = static_cast<RosterBaseItem *>(parentItem->child(i));
		if (osChildItem->itemType() == RosterBaseItem::RosterTypeContact)
		{
			OrgStructContactItem *osContactChildItem = static_cast<OrgStructContactItem *>(osChildItem);
			QString wid = osContactChildItem->itemWid();
			if (m_osContactItems.contains(wid))
			{
				QStandardItem *contactChildItem = m_osContactItems[wid];
				if (contactChildItem->checkState() == Qt::Checked)
				{
					hasChecked = true;
				}
				else
				{
					hasUnchecked = true;
				}
			}
		}
		else
		{
			OrgStructGroupItem *osGroupChildItem = static_cast<OrgStructGroupItem *>(osChildItem);
			QString groupId = osGroupChildItem->itemId();
			if (m_osGroupItems.contains(groupId))
			{
				QStandardItem *groupChildItem = m_osGroupItems[groupId];
				if (groupChildItem->checkState() == Qt::Checked)
				{
					hasChecked = true;
				}
				else if (groupChildItem->checkState() == Qt::Unchecked)
				{
					hasUnchecked = true;
				}
				else // groupChildItem->checkState() == Qt::PartiallyChecked
				{
					hasChecked = true;
					hasUnchecked = true;
				}
			}
		}

		if (hasChecked && hasUnchecked)
		{
			break;
		}
	}

	Qt::CheckState checkState = Qt::Checked;
	if (hasChecked && hasUnchecked)
	{
		checkState = Qt::PartiallyChecked;
	}
	else if (hasChecked)
	{
		checkState = Qt::Checked;
	}
	else
	{
		checkState = Qt::Unchecked;
	}

	QStandardItem *groupItem = m_osGroupItems[parentItem->itemId()];
	groupItem->setCheckState(checkState);

	// check the up level
	checkOsGroupChecked(groupItem);
}

void SelectContactWidget::selectRosterGroupChildren(QStandardItem *parentItem, bool select /*= true*/)
{
	if (!parentItem)
	{
		return;
	}

	QString groupName = parentItem->text();
	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	QStringList rosterIds = rosterModel->groupRosters(groupName);
	foreach (QString rosterId, rosterIds)
	{
		if (m_rosterContactItems.contains(rosterId))
		{
			QStandardItem *rosterItem = m_rosterContactItems[rosterId];

			if (select)
			{
				rosterItem->setCheckState(Qt::Checked);

				if (m_fixedIds.contains(rosterId))
				{
					rosterItem->setEnabled(false);
				}

				// add to dest model
				addDestItem(rosterItem, SelectRoster);
			}
			else
			{
				if (!m_fixedIds.contains(rosterId))
				{
					rosterItem->setCheckState(Qt::Unchecked);
					rosterItem->setEnabled(true);
					removeDestItem(rosterItem, SelectRoster);
				}
			}
		}
	}
}

void SelectContactWidget::selectOsGroupChildren(QStandardItem *parentItem, bool select /*= true*/)
{
	if (!parentItem)
	{
		return;
	}

	QString groupId = parentItem->data(IDRole).toString();
	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel *osModel = modelManager->orgStructModel();
	if (!osModel->containsGroup(groupId))
	{
		return;
	}

	if (!m_osGroupItems.contains(groupId))
	{
		return;
	}

	OrgStructGroupItem *osParentItem = osModel->orgStructGroup(groupId);
	for (int i = 0; i < osParentItem->rowCount(); i++)
	{
		RosterBaseItem *osChildItem = static_cast<RosterBaseItem *>(osParentItem->child(i));
		if (osChildItem->itemType() == RosterBaseItem::RosterTypeContact)
		{
			OrgStructContactItem *osContactChildItem = static_cast<OrgStructContactItem *>(osChildItem);
			QString wid = osContactChildItem->itemWid();
			if (m_osContactItems.contains(wid))
			{
				QStandardItem *contactChildItem = m_osContactItems[wid];

				if (select)
				{
					contactChildItem->setCheckState(Qt::Checked);

					if (m_fixedIds.contains(wid2Uid(wid)))
					{
						contactChildItem->setEnabled(false);
					}

					addDestItem(contactChildItem, SelectOS);
				}
				else
				{
					if (!m_fixedIds.contains(wid2Uid(wid)))
					{
						contactChildItem->setCheckState(Qt::Unchecked);
						contactChildItem->setEnabled(true);

						removeDestItem(contactChildItem, SelectOS);
					}
				}
			}
		}
		else
		{
			OrgStructGroupItem *osGroupChildItem = static_cast<OrgStructGroupItem *>(osChildItem);
			QString groupId = osGroupChildItem->itemId();
			if (m_osGroupItems.contains(groupId))
			{
				QStandardItem *groupChildItem = m_osGroupItems[groupId];
				if (select)
				{
					// do nothing, only select the contacts, do not select lower levels
					/*
					groupChildItem->setCheckState(Qt::Checked);

					// select lower levels
					selectOsGroupChildren(groupChildItem);
					*/
				}
				else
				{
					groupChildItem->setCheckState(Qt::Unchecked);

					// de-select lower levels
					selectOsGroupChildren(groupChildItem, false);
				}
			}
		}
	}
}

QStandardItem *SelectContactWidget::createSelectItem(bool checked, int type, const QString &id, const QString &name, 
													 int index, int groupIndex, const QString &groupName)
{
	QStandardItem *selectItem = 0;
	if (type == TypeRoleGroup)
	{
		selectItem = new SelectContactGroupItem(name);
	}
	else
	{
		selectItem = new QStandardItem(name);
	}
	if (checked)
	{
		selectItem->setCheckState(Qt::Checked);
	}
	else
	{
		selectItem->setCheckState(Qt::Unchecked);
	}
	selectItem->setData(type, TypeRole);
	selectItem->setData(id, IDRole);
	selectItem->setData(index, IndexRole);
	selectItem->setData(groupIndex, GroupIndexRole);
	selectItem->setData(groupName, GroupNameRole);
	return selectItem;
}

QString SelectContactWidget::wid2Uid(const QString &wid)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel *osModel = modelManager->orgStructModel();
	return osModel->wid2Uid(wid);
}

QStringList SelectContactWidget::uid2Wid(const QString &uid)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel *osModel = modelManager->orgStructModel();
	return osModel->uid2Wid(uid);
}

QString SelectContactWidget::makeDestSourceId(const QString &uid, SelectSource selectSource)
{
	if (selectSource == SelectRoster)
	{
		return QString("roster_%1").arg(uid);
	}
	else if (selectSource == SelectOS)
	{
		return QString("os_%1").arg(uid);
	}
	else
	{
		return QString();
	}
}

bool SelectContactWidget::hasRosterSelect() const
{
	foreach (QString destSourceId, m_destSourceItems.keys())
	{
		if (destSourceId.startsWith("roster_"))
		{
			return true;
		}
	}
	return false;
}

bool SelectContactWidget::hasOsContactSelect() const
{
	foreach (QString destSourceId, m_destSourceItems.keys())
	{
		if (destSourceId.startsWith("os_"))
		{
			return true;
		}
	}
	return false;
}

SelectContactWidget::SelectSource SelectContactWidget::selectSourceFromDestSourceId(const QString &destSourceId)
{
	if (destSourceId.startsWith("roster_"))
	{
		return SelectRoster;
	}
	else
	{
		return SelectOS;
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SelectContactItemDelegate
SelectContactItemDelegate::SelectContactItemDelegate(SelectContactWidget::SelectSource selectType, QObject *parent /*= 0*/)
	: QItemDelegate(parent), m_sourceType(selectType)
{
	m_avatar = QPixmap::fromImage(ModelManager::avatarDefaultSmallIcon());
	m_checkedNormal = QPixmap(":/theme/checkbox/checkbox_checked_normal.png");
	m_checkedDisabled = QPixmap(":/theme/checkbox/checkbox_checked_disabled.png");
	m_uncheckedNormal = QPixmap(":/theme/checkbox/checkbox_unchecked_normal.png");
	m_uncheckedDisabled = QPixmap(":/theme/checkbox/checkbox_unchecked_disabled.png");
	m_partiallyCheckedNormal = QPixmap(":/theme/checkbox/checkbox_indeterminate_normal.png");
	m_partiallyCheckedDisabled = QPixmap(":/theme/checkbox/checkbox_indeterminate_disabled.png");
	m_groupExpand = QPixmap(":/images/Icon_42.png");
	m_groupCollapse = QPixmap(":/images/Icon_43.png");
}

QSize SelectContactItemDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
	return QSize(30, 30);
}

void SelectContactItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(painter);

	QTreeView *view = static_cast<QTreeView *>(parent());
	if (!view)
	{
		return;
	}

	QModelIndex sourceIndex = index;
	if (sourceIndex.data(TypeRole).toInt() == TypeRoleGroup)
	{
		QWidget *w = view->indexWidget(sourceIndex);
		if (w)
		{
			w->setGeometry(QRect(option.rect));
		}
		else
		{
			drawGroupItem(painter, option, index);
		}
	}
	else
	{
		drawContactItem(painter, option, index);
	}
}

bool SelectContactItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if (m_sourceType != SelectContactWidget::SelectOS)
		return false;

	if (!index.isValid())
		return false;

	QStandardItem *item = ((QStandardItemModel *)(model))->itemFromIndex(index);
	if (!item)
		return false;

	if (!item->isEnabled())
		return false;

	if (index.data(TypeRole).toInt() != TypeRoleGroup) // Contact
	{
		if (event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			if (!mouseEvent)
				return false;

			if (item->checkState() == Qt::Unchecked)
			{
				item->setCheckState(Qt::Checked);
			}
			else if (item->checkState() == Qt::Checked)
			{
				item->setCheckState(Qt::Unchecked);
			}

			emit itemToggled(item, (item->checkState() == Qt::Checked));

			return true;
		}

		return false;
	}

	SelectContactGroupItem *groupItem = static_cast<SelectContactGroupItem *>(item);
	if (!groupItem->checkEnabled())
		return false;

	if (event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if (!mouseEvent)
			return false;

		QPoint pt = mouseEvent->pos();
		QRect checkRect = option.rect;
		checkRect.setLeft(checkRect.left()+2+m_groupExpand.width()+1);
		checkRect.setTop(checkRect.top()+(checkRect.height()-m_checkedNormal.height())/2);
		checkRect.setSize(m_checkedNormal.size());

		// qDebug() << "------" << index << pt << checkRect << checkRect.contains(pt);

		if (checkRect.contains(pt))
		{
			if (groupItem->checkState() == Qt::Unchecked)
			{
				groupItem->setCheckState(Qt::Checked);
			}
			else if (groupItem->checkState() == Qt::Checked)
			{
				groupItem->setCheckState(Qt::Unchecked);
			}
			else
			{
				groupItem->setCheckState(Qt::Unchecked);
			}

			emit itemToggled(groupItem, (groupItem->checkState() == Qt::Checked));
			
			return true;
		}
		else
		{
			emit clicked(groupItem->index());

			return true;
		}
	}

	return false;
}

void SelectContactItemDelegate::drawGroupItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStandardItem *item = ((QStandardItemModel *)(index.model()))->itemFromIndex(index);
	if (!item)
		return;

	SelectContactGroupItem *groupItem = static_cast<SelectContactGroupItem *>(item);
	if (!groupItem)
		return;

	QString name = groupItem->text();
	QRect rect = option.rect;

	// draw group expand
	QPoint pt = rect.topLeft();
	pt.setX(pt.x()+2);
	pt.setY(rect.top()+(rect.height()-m_groupExpand.height())/2);
	if (groupItem->groupExpanded())
	{
		painter->drawPixmap(pt, m_groupExpand);
	}
	else
	{
		painter->drawPixmap(pt, m_groupCollapse);
	}

	// draw check
	QPixmap checkImg;
	if (groupItem->checkEnabled())
	{
		if (groupItem->checkState() == Qt::Checked)
		{
			checkImg = m_checkedNormal;
		}
		else if (groupItem->checkState() == Qt::PartiallyChecked)
		{
			checkImg = m_partiallyCheckedNormal;
		}
		else
		{
			checkImg = m_uncheckedNormal;
		}
	}
	else
	{
		if (groupItem->checkState() == Qt::Checked)
		{
			checkImg = m_checkedDisabled;
		}
		else if (groupItem->checkState() == Qt::PartiallyChecked)
		{
			checkImg = m_partiallyCheckedDisabled;
		}
		else
		{
			checkImg = m_uncheckedDisabled;
		}
	}
	pt.setX(pt.x()+m_groupExpand.width()+1);
	pt.setY(rect.top()+(rect.height()-checkImg.height())/2);
	painter->drawPixmap(pt, checkImg);

	// draw name
	pt.setX(pt.x()+checkImg.width()+2);
	QString text = name;
	painter->setPen(QColor(0, 0, 0));
	if (groupItem->isLoading())
	{
		text = tr("loading...");
		painter->setPen(QColor(128, 128, 128));
	}
	rect = option.rect;
	rect.setLeft(pt.x());
	name = option.fontMetrics.elidedText(text, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, text);
}

void SelectContactItemDelegate::drawContactItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QModelIndex sourceIndex = index;
	QStandardItem *contactItem = ((QStandardItemModel *)(sourceIndex.model()))->itemFromIndex(sourceIndex);
	if (!contactItem)
		return;

	QString id = contactItem->data(IDRole).toString();
	QString name = contactItem->text();
	QString tailText;
	if (m_sourceType == SelectContactWidget::SelectOS)
	{
		OrgStructModel *osModel = qPmApp->getModelManager()->orgStructModel();
		OrgStructContactItem *osContact = osModel->contactByWid(id);
		if (osContact)
		{
			if (osContact->itemUserState() == OrgStructContactItem::USER_STATE_INACTIVE)
				tailText = tr("(unregistered)");
		}
	}

	QRect rect = option.rect;

	// draw check
	QPixmap checkImg;
	if (contactItem->isEnabled())
	{
		if (contactItem->checkState() == Qt::Checked)
		{
			checkImg = m_checkedNormal;
		}
		else
		{
			checkImg = m_uncheckedNormal;
		}
	}
	else
	{
		if (contactItem->checkState() == Qt::Checked)
		{
			checkImg = m_checkedDisabled;
		}
		else
		{
			checkImg = m_uncheckedDisabled;
		}
	}
	rect = option.rect;
	rect.setWidth(checkImg.width());
	rect.setHeight(checkImg.height());
	rect.moveTop(rect.y() + (option.rect.height() - checkImg.height())/2);
	rect.moveLeft(rect.x() + 3);
	painter->drawPixmap(rect.topLeft(), checkImg);

	// draw avatar
	QPixmap avatar = m_avatar;
	QSize avatarSize = m_avatar.size();
	rect = option.rect;
	rect.setWidth(avatarSize.width());
	rect.setHeight(avatarSize.height());
	rect.moveTop(rect.y() + (option.rect.height() - avatarSize.height())/2);
	rect.moveLeft(rect.x() + 3 + checkImg.width() + 5);
	QBitmap avatarMask(":/images/Icon_60_mask20.png");
	avatar.setMask(avatarMask);
	painter->drawPixmap(rect.topLeft(), avatar);

	// draw name
	painter->setPen(QColor(0, 0, 0));
	if (!tailText.isEmpty())
	{
		painter->setPen(QColor(136, 136, 136));
		name += tailText;
	}

	rect = option.rect;
	rect.setLeft(rect.left() + 3 + checkImg.width() + 5 + avatarSize.width() + 5);
	rect.setTop(rect.top() + 3);
	rect.setBottom(rect.bottom() - 3);

	name = option.fontMetrics.elidedText(name, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, name);
}

SelectRosterContactItemDelegate::SelectRosterContactItemDelegate(SelectContactWidget::SelectSource selectType, QObject *parent /*= 0*/)
	: SelectContactItemDelegate(selectType, parent)
{
}

QSize SelectRosterContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	QModelIndex sourceIndex = index;
	if (sourceIndex.data(TypeRole).toInt() == TypeRoleGroup)
	{
		return QSize(22, 22);
	}
	else
	{
		return QSize(30, 30);
	}
}

void SelectRosterContactItemDelegate::drawGroupItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// fill item background color
	QRect paintRect = option.rect;
	painter->fillRect(paintRect, QColor("#e6e6e6"));

	// draw name
	painter->save();
	QFont font = painter->font();
	font.setPointSize(9);
	painter->setFont(font);

	QString groupName = index.data(Qt::DisplayRole).toString();

	QFontMetrics fm = option.fontMetrics;
	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + 5);
	paintRect.setRight(paintRect.right() - 68);
	QString groupText = fm.elidedText(groupName, Qt::ElideMiddle, paintRect.width());
	painter->setPen(QColor("#808080"));
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, groupText);

	painter->restore();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SelectTreeItemDelegate
SelectContactDestItemDelegate::SelectContactDestItemDelegate(QObject *parent /*= 0*/)
: QItemDelegate(parent)
{
	m_avatar = QPixmap::fromImage(ModelManager::avatarDefaultSmallIcon());
}

QSize SelectContactDestItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	return QSize(30, 30);
}

void SelectContactDestItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (!index.isValid())
		return;

	painter->save();
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);
	QRect paintRect = option.rect;
	TypeRoleType itemType = static_cast<TypeRoleType>(index.data(TypeRole).toInt());
	if (itemType == TypeRoleContact)
	{
		drawContactItem(painter, option, index);
	}

	painter->restore();
}

bool SelectContactDestItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	QListView *listView = qobject_cast<QListView *>(this->parent());
	if (!listView)
		return false;

	QModelIndex sourceIndex = index;
	QStandardItem *contactItem = ((QStandardItemModel *)(model))->itemFromIndex(sourceIndex);
	if (!contactItem)
		return false;

	if (!contactItem->isEnabled())
		return false;

	if (event->type() == QEvent::MouseMove)
	{
		if (m_delButtons.contains(m_lastId))
		{
			m_delButtons[m_lastId]->setVisible(false);
		}

		QString id = contactItem->data(IDRole).toString();
		QRect rect = option.rect;
		StyleToolButton *button = 0;
		if (!m_delButtons.contains(id))
		{
			button = new StyleToolButton(listView);
			StyleToolButton::Info info;
			info.urlNormal = ":/images/fm_delfile_hover.png";
			info.urlHover = ":/images/fm_delfile_hover.png";
			info.urlPressed = ":/images/fm_delfile_pressed.png";
			info.urlDisabled = "";
			info.tooltip = "";
			button->setInfo(info);
			m_delButtons[id] = button;
			connect(button, SIGNAL(clicked()), this, SLOT(onDelButtonClicked()));
		}
		else
		{
			button = (StyleToolButton *)(m_delButtons[id]);
		}

		int buttonHeight = 20;
		if (listView->verticalScrollBar())
		{
			if (listView->verticalScrollBar()->isVisible())
			{
				button->setGeometry(rect.x()+rect.width()+listView->verticalScrollBar()->width()-buttonHeight-30, 
					rect.y()+(rect.height()-buttonHeight)/2, buttonHeight, buttonHeight);
			}
			else
			{
				button->setGeometry(rect.x()+rect.width()-buttonHeight-30, rect.y()+(rect.height()-buttonHeight)/2, 
					buttonHeight, buttonHeight);
			}
		}
		else
		{
			button->setGeometry(rect.x()+rect.width()-buttonHeight-30, rect.y()+(rect.height()-buttonHeight)/2, 
				buttonHeight, buttonHeight);
		}
		button->setVisible(true);

		m_lastId = id;

		// qDebug() << "----------------------------------------- EDIT MOUSE MOVE" << id << option.rect << index.row();

		return true;
	}

	return false;
}

void SelectContactDestItemDelegate::onDelButtonClicked()
{
	QWidget *clickedButton = (QWidget *)sender();
	if (!clickedButton)
		return;

	QString id = m_delButtons.key(clickedButton);
	if (id.isEmpty())
		return;

	// qDebug() << "----------------------------------------- DEL BUTTON CLICKED:" << id;

	m_delButtons.remove(id);
	clickedButton->deleteLater();

	emit delButtonClicked(id);
}

void SelectContactDestItemDelegate::drawContactItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QModelIndex sourceIndex = index;
	QStandardItem *contactItem = ((QStandardItemModel *)(sourceIndex.model()))->itemFromIndex(sourceIndex);
	if (!contactItem)
		return;

	QString id = contactItem->data(IDRole).toString();
	QString name = contactItem->text();
	QRect rect = option.rect;

	if (!(option.state & QStyle::State_MouseOver))
	{
		if (m_delButtons.contains(id))
			m_delButtons[id]->setVisible(false);
	}
	else
	{
		painter->fillRect(rect, QColor(223, 238, 250));
	}

	// draw avatar
	QPixmap avatar = m_avatar;
	QSize avatarSize = m_avatar.size();
	rect = option.rect;
	rect.setWidth(avatarSize.width());
	rect.setHeight(avatarSize.height());
	rect.moveTop(rect.y() + (option.rect.height() - avatarSize.height())/2);
	rect.moveLeft(rect.x() + 9);
	QBitmap avatarMask(":/images/Icon_60_mask20.png");
	avatar.setMask(avatarMask);
	painter->drawPixmap(rect.topLeft(), avatar);

	// draw name
	if (contactItem->isEnabled())
		painter->setPen(QColor(0, 0, 0));
	else
		painter->setPen(QColor(96, 96, 96));

	rect = option.rect;
	rect.setLeft(rect.left() + 9 + avatarSize.width() + 4);
	rect.setTop(rect.top() + 3);
	rect.setBottom(rect.bottom() - 3);

	name = option.fontMetrics.elidedText(name, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, name);
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RosterHeaderView
SelectContactSourceHV::SelectContactSourceHV(SelectContactWidget *widget, QStandardItemModel *sourceModel, QWidget *parent)
: QHeaderView(Qt::Horizontal, parent), m_widget(widget), m_sourceModel(sourceModel), m_groupItem(0)
{
	setStyleSheet(QString("QHeaderView::section {"
		"font-size: 10.5pt;"
		"color: black;"
		"background-color: rgb(240, 240, 240);"
		"padding-left: 0px;"
		"border: none;"
		"border-bottom: 3px solid qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(231, 231, 231), stop: 1.0 rgb(249, 249, 249));"
		"}"
		));

	setSectionResizeMode(QHeaderView::Stretch);
	setSortIndicatorShown(false);
	setSectionsClickable(true);
	setFixedHeight(30);

	connect(this, SIGNAL(sectionClicked(int)), this, SLOT(onSectionClicked(int)));
}

void SelectContactSourceHV::setGroupItem(QStandardItem *item)
{
	m_groupItem = item;
	if (m_groupItem)
	{
		QStandardItem *headerItem = m_sourceModel->horizontalHeaderItem(0);
		if (headerItem)
		{
			QString groupName = m_groupItem->text();

			/* // show number
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(m_groupItem->index());
			int number = m_proxyModel->rowCount(proxyIndex);
			QString numberText = QString("[%1]").arg(number); 
			QString headerText = QString("%1 %2").arg(groupName).arg(numberText);
			*/

			QString headerText = groupName;
			QStandardItem *itemParent = static_cast<QStandardItem *>(m_groupItem->parent());
			while (itemParent)
			{
				headerText.insert(0, itemParent->text() + " | ");
				itemParent = static_cast<QStandardItem *>(itemParent->parent());
			}

			int elideWidth = width() - 16 - 6 - 6;
			QTreeView *treeView = static_cast<QTreeView *>(parent());
			if (treeView)
			{
				if (treeView->verticalScrollBar() && treeView->verticalScrollBar()->isVisible())
				{
					elideWidth -= treeView->verticalScrollBar()->width();
				}
			}

			QFontMetrics fm = fontMetrics();
			headerText = fm.elidedText(headerText, Qt::ElideLeft, elideWidth);
			headerItem->setText(headerText);

			setSortIndicatorShown(false);
			setSectionsClickable(true);
			update();
		}
	}
}

void SelectContactSourceHV::onSectionClicked(int index)
{
	Q_UNUSED(index);

	if (!m_groupItem)
		return;

	if (!isVisible())
		return;

	QTreeView *treeView = static_cast<QTreeView *>(parent());
	if (!treeView)
		return;

	// get group index
	QModelIndex sourceIndex = m_groupItem->index();
	SelectContactGroupItem *groupItem = static_cast<SelectContactGroupItem *>(m_groupItem);
	if (groupItem)
	{
		groupItem->setGroupExpanded(false);
	}

	// make header view invisible
	setGroupItem(0);
	m_widget->setHeaderVisible(this, false);

	// collapse group item and scroll to it
	treeView->collapse(sourceIndex);
	treeView->scrollTo(sourceIndex);
}


