#include "selectchatwidget.h"
#include "ui_selectchatwidget.h"

#include <QDebug>
#include <QItemDelegate>
#include <QPainter>
#include <QStringList>
#include <QBitmap>

#include "model/ModelManager.h"
#include "model/rostermodeldef.h"
#include "model/groupitemdef.h"
#include "model/orgstructitemdef.h"
#include "model/groupmodeldef.h"
#include "model/orgstructmodeldef.h"
#include "model/DiscussModeldef.h"
#include "model/DiscussItemdef.h"
#include "model/lastcontactmodeldef.h"
#include "model/lastcontactitemdef.h"
#include "PmApp.h"
#include "editfiltertreeview.h"
#include "editfilterlistview.h"
#include "Account.h"
#include "Constants.h"

static const int TypeRole = Qt::UserRole + 1024;
static const int IDRole    = TypeRole  + 1;
static const int NameRole  = TypeRole  + 2;
static const int IndexRole = TypeRole  + 3;
static const int CountRole = TypeRole  + 4;

//////////////////////////////////////////////////////////////////////////
// CLASS SelectChatTreeModel
class SelectChatTreeModel : public QStandardItemModel
{
	Q_OBJECT

public:
	explicit SelectChatTreeModel(bool showLastContact = false, QObject *parent = 0);
	virtual ~SelectChatTreeModel();

	QStandardItem *rosterGroup() const;
	QStandardItem *osGroup() const;
	QStandardItem *groupGroup() const;
	QStandardItem *discussGroup() const;
	QStandardItem *lastContactGroup() const;

private:
	void init(bool showLastContact);

private:

	QStandardItem                  *m_rosterGroup;
	QStandardItem                  *m_osGroup;
	QStandardItem                  *m_groupGroup;
	QStandardItem                  *m_discussGroup;
	QStandardItem                  *m_lastContactGroup;
};

SelectChatTreeModel::SelectChatTreeModel(bool showLastContact /*= false*/, QObject *parent /*= 0*/)
	: QStandardItemModel(parent)
	, m_rosterGroup(0)
	, m_osGroup(0)
	, m_groupGroup(0)
	, m_discussGroup(0)
	, m_lastContactGroup(0)
{
	init(showLastContact);
}

SelectChatTreeModel::~SelectChatTreeModel()
{

}

QStandardItem *SelectChatTreeModel::rosterGroup() const
{
	return m_rosterGroup;
}

QStandardItem *SelectChatTreeModel::osGroup() const
{
	return m_osGroup;
}

QStandardItem *SelectChatTreeModel::groupGroup() const
{
	return m_groupGroup;
}

QStandardItem *SelectChatTreeModel::discussGroup() const
{
	return m_discussGroup;
}

QStandardItem *SelectChatTreeModel::lastContactGroup() const
{
	return m_lastContactGroup;
}

void SelectChatTreeModel::init(bool showLastContact)
{
	ModelManager* pModelManager = qPmApp->getModelManager();

	// init tree model
	int groupIndex = 0;

	if (showLastContact)
	{
		LastContactModel *lastContactModel = pModelManager->lastContactModel();
		QStringList allContactIds = lastContactModel->allContactIds();
		allContactIds.removeAll(QString(SUBSCRIPTION_ROSTER_ID));
		QStringList allGroupIds = lastContactModel->allMucGroupIds();
		QStringList allDiscussIds = lastContactModel->allDiscussIds();
		int lastCount = allContactIds.count() + allGroupIds.count() + allDiscussIds.count();
		if (lastCount > 0)
		{
			QStandardItem *pLastContactItem = new QStandardItem();
			pLastContactItem->setData(SelectChatWidget::LastContactSource, TypeRole);
			pLastContactItem->setData("lastcontact0", IDRole);
			pLastContactItem->setData(groupIndex++, IndexRole);
			pLastContactItem->setText(tr("Recently contact"));
			// pLastContactItem->setData(lastCount, CountRole);
			pLastContactItem->setFlags(pLastContactItem->flags() & ~Qt::ItemIsEditable);
			invisibleRootItem()->appendRow(pLastContactItem);
			m_lastContactGroup = pLastContactItem;
		}
	}

	// add roster contact
	QStandardItem *pRoster = new QStandardItem(tr("Friends"));
	pRoster->setData(SelectChatWidget::RosterSource, TypeRole);
	pRoster->setData("roster0", IDRole);
	pRoster->setData(groupIndex++, IndexRole);
	pRoster->setFlags(pRoster->flags() & ~Qt::ItemIsEditable);
	invisibleRootItem()->appendRow(pRoster);
	m_rosterGroup = pRoster;

	// add os contact
	QStandardItem *pOs = new QStandardItem(tr("Corporation"));
	pOs->setData(SelectChatWidget::OsSource, TypeRole);
	pOs->setData("orgstruct0", IDRole);
	pOs->setData(groupIndex++, IndexRole);
	pOs->setFlags(pOs->flags() & ~Qt::ItemIsEditable);
	invisibleRootItem()->appendRow(pOs);
	m_osGroup = pOs;

	// add groups
	// QStringList groupIds = pModelManager->groupModel()->allGroupIds();
	QStandardItem *pGroupItem = new QStandardItem();
	pGroupItem->setData(SelectChatWidget::GroupSource, TypeRole);
	pGroupItem->setData("group0", IDRole);
	pGroupItem->setData(groupIndex++, IndexRole);
	pGroupItem->setText(tr("Groups"));
	// pGroupItem->setData(groupIds.count(), CountRole);
	pGroupItem->setFlags(pGroupItem->flags() & ~Qt::ItemIsEditable);
	invisibleRootItem()->appendRow(pGroupItem);
	m_groupGroup = pGroupItem;

	// add discusses
	// QStringList discussIds = pModelManager->discussModel()->allDiscussIds();
	QStandardItem *pDiscussItem = new QStandardItem();
	pDiscussItem->setData(SelectChatWidget::DiscussSource, TypeRole);
	pDiscussItem->setData("discuss0", IDRole);
	pDiscussItem->setData(groupIndex++, IndexRole);
	pDiscussItem->setText(tr("Discusses"));
	// pDiscussItem->setData(discussIds.count(), CountRole);
	pDiscussItem->setFlags(pDiscussItem->flags() & ~Qt::ItemIsEditable);
	invisibleRootItem()->appendRow(pDiscussItem);
	m_discussGroup = pDiscussItem;
}

//////////////////////////////////////////////////////////////////////////
// CLASS SelectChatListModel
class SelectChatListModel : public QStandardItemModel
{
	Q_OBJECT
public:
	explicit SelectChatListModel(QObject *parent = 0);
	virtual ~SelectChatListModel();

	QStandardItem *getChatListItem(const QString &id) const;

public:
	void change(SelectChatWidget::Source source);

private:
	int addOsListItem(OrgStructGroupItem *pOrgGroup, int &index);

private:
	QMap<QString, QStandardItem *> m_mapItems;
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

SelectChatListModel::SelectChatListModel(QObject *parent /*= 0*/)
	: QStandardItemModel(parent)
{

}

SelectChatListModel::~SelectChatListModel()
{

}

QStandardItem * SelectChatListModel::getChatListItem(const QString &id) const
{
	return m_mapItems.value(id, 0);
}

void SelectChatListModel::change(SelectChatWidget::Source source)
{
	clear();
	m_mapItems.clear();

	int count = 0;
	ModelManager *pModelManager = qPmApp->getModelManager();

	switch (source)
	{
	case SelectChatWidget::LastContactSource:
		{
			int lastContactIndex = 0;
			LastContactModel *pLastContactModel = pModelManager->lastContactModel();
			int rowCount = pLastContactModel->rowCount();
			for (int i = 0; i < rowCount; ++i)
			{
				LastContactItem *lastContactItem = pLastContactModel->nodeFromRow(i);
				LastContactItem::LastContactItemType lastContactType = lastContactItem->itemType();
				QString lastContactId = lastContactItem->itemId();
				QString name = lastContactItem->itemName();
				
				if (lastContactType == LastContactItem::LastContactTypeContact)
				{
					if (lastContactId != QString(SUBSCRIPTION_ROSTER_ID))
					{
						QStandardItem *pItem = new QStandardItem();
						pItem->setText(name);
						pItem->setData(name, NameRole);
						pItem->setData(SelectChatWidget::RosterSource, TypeRole);
						pItem->setData(lastContactId, IDRole);
						pItem->setData(lastContactIndex++, IndexRole);
						pItem->setToolTip(name);

						invisibleRootItem()->appendRow(pItem);
						m_mapItems[lastContactId] = pItem;

						++count;
					}
				}
				else if (lastContactType == LastContactItem::LastContactTypeGroupMuc)
				{
					QStandardItem *pItem = new QStandardItem();
					pItem->setText(name);
					pItem->setData(name, NameRole);
					pItem->setData(SelectChatWidget::GroupSource, TypeRole);
					pItem->setData(lastContactId, IDRole);
					pItem->setData(lastContactIndex++, IndexRole);
					pItem->setToolTip(name);

					invisibleRootItem()->appendRow(pItem);
					m_mapItems[lastContactId] = pItem;

					++count;
				}
				else if (lastContactType == LastContactItem::LastContactTypeDiscuss)
				{
					QStandardItem *pItem = new QStandardItem();
					pItem->setText(name);
					pItem->setData(name, NameRole);
					pItem->setData(SelectChatWidget::DiscussSource, TypeRole);
					pItem->setData(lastContactId, IDRole);
					pItem->setData(lastContactIndex++, IndexRole);
					pItem->setToolTip(name);

					invisibleRootItem()->appendRow(pItem);
					m_mapItems[lastContactId] = pItem;

					++count;
				}
			}
		}
		break;

	case SelectChatWidget::RosterSource:
		{
			RosterModel *pRosterModel = pModelManager->rosterModel();
			QStringList rosterIds = pRosterModel->allRosterIds();

			// init list model
			int rosterIndex = 0;
			QString rosterName;
			foreach (QString rosterId, rosterIds)
			{
				rosterName = pRosterModel->rosterName(rosterId);
				QStandardItem *pItem = new QStandardItem();
				pItem->setText(rosterName);
				pItem->setData(rosterName, NameRole);
				pItem->setData(SelectChatWidget::RosterSource, TypeRole);
				pItem->setData(rosterId, IDRole);
				pItem->setData(rosterIndex++, IndexRole);
				pItem->setToolTip(rosterName);

				invisibleRootItem()->appendRow(pItem);
				m_mapItems[rosterId] = pItem;

				++count;
			}
		}
		break;

	case SelectChatWidget::OsSource:
		{
			OrgStructModel *pOrgModel = pModelManager->orgStructModel();
			int index = 0;
			foreach (QString groupid, pOrgModel->topGroupIds())
			{	
				OrgStructGroupItem *pOrgGroup = pOrgModel->orgStructGroup(groupid);
				if (!pOrgGroup)
					continue;

				addOsListItem(pOrgGroup, index);
			}
		}
		break;

	case SelectChatWidget::GroupSource:
		{
			// init list model
			foreach (QString id, pModelManager->groupModel()->allGroupIds())
			{
				MucGroupItem *pMucItem = pModelManager->groupModel()->getGroup(id);

				QStandardItem *pItem = new QStandardItem();
				pItem->setText(pMucItem->itemName());
				pItem->setData(pMucItem->itemName(), NameRole);
				pItem->setData(SelectChatWidget::GroupSource, TypeRole);
				pItem->setData(pMucItem->itemId(), IDRole);
				pItem->setData(pMucItem->itemIndex(), IndexRole);
				pItem->setToolTip(pMucItem->itemName());

				invisibleRootItem()->appendRow(pItem);
				m_mapItems[pMucItem->itemId()] = pItem;

				++count;
			}
		}
		break;

	case SelectChatWidget::DiscussSource:
		{
			// init list model
			int discussIndex = 0;
			foreach (QString id, pModelManager->discussModel()->allDiscussIds())
			{
				DiscussItem *pDiscussItem = pModelManager->discussModel()->getDiscuss(id);

				QStandardItem *pItem = new QStandardItem();
				pItem->setText(pDiscussItem->itemName());
				pItem->setData(pDiscussItem->itemName(), NameRole);
				pItem->setData(SelectChatWidget::DiscussSource, TypeRole);
				pItem->setData(pDiscussItem->itemId(), IDRole);
				pItem->setData(discussIndex++, IndexRole);
				pItem->setToolTip(pDiscussItem->itemName());

				invisibleRootItem()->appendRow(pItem);
				m_mapItems[pDiscussItem->itemId()] = pItem;

				++count;
			}
		}
		break;

	default:
		break;
	}

	sort(0);
}

int SelectChatListModel::addOsListItem(OrgStructGroupItem *pOrgGroup, int &index)
{
	int count = 0;

	if (!pOrgGroup)
	{
		return count;
	}

	QList<RosterBaseItem *> osChildItems;
	for (int i = 0; i < pOrgGroup->rowCount(); i++)
	{
		RosterBaseItem *item = static_cast<RosterBaseItem *>(pOrgGroup->child(i));
		osChildItems.append(item);
	}

	qSort(osChildItems.begin(), osChildItems.end(), osSourceLessThan);

	foreach (RosterBaseItem *osItem, osChildItems)
	{
		if (osItem->itemType() == RosterBaseItem::RosterTypeGroup)
		{
			OrgStructGroupItem *childGroup = static_cast<OrgStructGroupItem *>(osItem);
			count += addOsListItem(childGroup, index);
		}
		else
		{
			OrgStructContactItem *pContact = static_cast<OrgStructContactItem *>(osItem);
			QString wid = pContact->itemWid();

			QStandardItem *pItem = new QStandardItem();
			pItem->setText(pContact->itemName());
			pItem->setData(pContact->itemName(), NameRole);
			pItem->setData(SelectChatWidget::OsSource, TypeRole);
			pItem->setData(wid, IDRole);
			pItem->setData(++index, IndexRole);
			QString tooltip = QString("%1 %2").arg(pContact->itemName()).arg(pOrgGroup->itemName());
			pItem->setToolTip(tooltip);

			invisibleRootItem()->appendRow(pItem);
			m_mapItems[wid] = pItem;

			++count;
		}
	}

	return count;
}

//////////////////////////////////////////////////////////////////////////
// CLASS TreeDelegate
class TreeDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	TreeDelegate(QObject *parent);
	virtual ~TreeDelegate();

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

TreeDelegate::TreeDelegate(QObject *parent)
	: QItemDelegate(parent)
{

}

TreeDelegate::~TreeDelegate()
{

}

QSize TreeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	return QSize(32, 32);
}

void TreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QTreeView *view = static_cast<QTreeView *>(parent());
	if (!view)
	{
		return;
	}

	painter->save();
		
	// draw background
	QRect paintRect = option.rect;
	QColor textColor;
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, QColor(247, 247, 247));
		textColor = QColor(0, 120, 216);

		QRect selRect = paintRect;
		selRect.setWidth(4);
		painter->fillRect(selRect, QColor(0, 120, 216));
	}
	else if (option.state & QStyle::State_MouseOver)
	{
		textColor = QColor(0, 120, 216);
	}
	else
	{
		textColor = QColor("#000000");
	}

	// draw text
	painter->setPen(textColor);
	QString text = index.data(Qt::DisplayRole).toString();
	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + 20);
	QFontMetrics fontMetrics = option.fontMetrics;
	text = fontMetrics.elidedText(text, Qt::ElideRight, paintRect.width());
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, text);

	painter->restore();
}

//////////////////////////////////////////////////////////////////////////
// CLASS ListDelegate
class ListDelegate : public QItemDelegate
{
	Q_OBJECT
public:
	ListDelegate(QObject *parent);
	~ListDelegate();

public:
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	QPixmap m_contactAvatar;
};

ListDelegate::ListDelegate(QObject *parent)
	: QItemDelegate(parent)
{
	m_contactAvatar = QPixmap::fromImage(ModelManager::avatarDefaultSmallIcon());
}

ListDelegate::~ListDelegate()
{

}

QSize ListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	return QSize(32, 32);
}

void ListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	QColor textColor(Qt::black);
	QColor deptColor(128, 128, 128);
	QColor bgColor;

	// draw background
	QRect paintRect = option.rect;
	if (option.state & QStyle::State_Selected)
	{
		bgColor = Qt::white;
		textColor = Qt::black;
	}
	else if (option.state & QStyle::State_MouseOver)
	{
		bgColor = QColor(247, 247, 247);
		textColor = QColor(0, 120, 216);
	}
	else 
	{
		bgColor = QColor(247, 247, 247);
		textColor = Qt::black;
	}

	painter->fillRect(paintRect, bgColor);

	// draw icon
	ModelManager *modelManager = qPmApp->getModelManager();
	int type = index.data(TypeRole).toInt();
	QString id = index.data(IDRole).toString();
	OrgStructModel *orgModel = qPmApp->getModelManager()->orgStructModel();
	QString uid = orgModel->wid2Uid(id);

	QPixmap avatar;
	QSize avatarSize(20, 20);

	switch (type)
	{
	case SelectChatWidget::RosterSource:
	case SelectChatWidget::OsSource:
		{
			avatar = m_contactAvatar;
			if (id == Account::instance()->phoneFullId())
				avatar = QPixmap(":/images/myphone.png").scaled(avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
		break;
	case SelectChatWidget::GroupSource:
		{
			avatar = modelManager->getGroupLogo(id).scaled(avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
		break;
	case SelectChatWidget::DiscussSource:
		{
			avatar = QPixmap(":/images/Icon_64_small.png");
		}
		break;
	default:
		return;
	}

	paintRect = option.rect;

	paintRect.setTop(paintRect.top() + (paintRect.height() - avatar.height())/2);
	paintRect.setLeft(paintRect.left() + 10);
	paintRect.setHeight(avatar.height());
	paintRect.setWidth(avatar.width());
	QBitmap avatarMask(":/images/Icon_60_mask20.png");
	avatar.setMask(avatarMask);
	painter->drawPixmap(paintRect, avatar);

	// draw text
	painter->setPen(textColor);
	QString text = index.data(Qt::DisplayRole).toString();
	QString dept;

	if (type == SelectChatWidget::OsSource)
	{
		OrgStructContactItem *osContact = orgModel->contactByWid(id);
		if (osContact)
		{
			dept = osContact->parentName();
			if (osContact->itemUserState() == OrgStructContactItem::USER_STATE_INACTIVE)
			{
				text.append(tr("(unregistered)"));
				painter->setPen(QColor(136, 136, 136));
			}
		}
	}

	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + avatar.width() + 15);
	if (!dept.isEmpty())
	{
		paintRect.setWidth(128);
	}
	else
	{
		paintRect.setRight(option.rect.right() - 9);
	}
	QFontMetrics fontMetrics = option.fontMetrics;
	text = fontMetrics.elidedText(text, Qt::ElideMiddle, paintRect.width());
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, text);

	if (!dept.isEmpty())
	{
		paintRect.setLeft(paintRect.right() + 5);
		paintRect.setRight(option.rect.right() - 9);
		painter->setPen(deptColor);
		dept = fontMetrics.elidedText(dept, Qt::ElideMiddle, paintRect.width());
		painter->drawText(paintRect, Qt::AlignRight|Qt::AlignVCenter, dept);
	}

	painter->restore();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SelectChatWidget
SelectChatWidget::SelectChatWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::SelectChatWidget();
	ui->setupUi(this);

	// signals
	connect(ui->treeViewGroups, SIGNAL(clicked(QModelIndex)), this, SLOT(onTreeViewGroupsClicked(QModelIndex)));
	connect(ui->listViewMembers, SIGNAL(clicked(QModelIndex)), this, SLOT(onListViewMembersActivated(QModelIndex)));
	connect(ui->listViewMembers, SIGNAL(clicked(QModelIndex)), this, SLOT(onListViewMembersClicked(QModelIndex)));
	connect(ui->searchPage, SIGNAL(selectSearchItem(QString, int, QString)), this, SLOT(editFilterSelected(QString, int, QString)));

	// set style
	ui->listViewMembers->setStyleSheet("QListView#listViewMembers {background-color: rgb(247, 247, 247); border: none;}");
	ui->treeViewGroups->setStyleSheet("QTreeView#treeViewGroups {background-color: rgb(232, 232, 232); border: none;}");
	ui->searchPage->setStyleSheet("QWidget#searchPage {border: none; border-right: 1px solid rgb(236, 236, 236);}");
}

SelectChatWidget::~SelectChatWidget()
{
	delete ui;
}

void SelectChatWidget::init(const QString &searchTip, bool showLastContact /*= false*/)
{
	TreeDelegate *treeDelegate = new TreeDelegate(ui->treeViewGroups);
	m_selectTreeModel = new SelectChatTreeModel(showLastContact);
	m_selectTreeModel->setSortRole(IndexRole);
	m_selectTreeModel->sort(0);
	ui->treeViewGroups->setModel(m_selectTreeModel);
	ui->treeViewGroups->setItemDelegate(treeDelegate);

	ListDelegate *listDelegate = new ListDelegate(ui->listViewMembers);
	m_selectListModel = new SelectChatListModel(ui->listViewMembers);
	m_selectListModel->setSortRole(IndexRole);
	ui->listViewMembers->setModel(m_selectListModel);
	ui->listViewMembers->setItemDelegate(listDelegate);
	ui->listViewMembers->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->treeViewGroups->setRootIsDecorated(false);
	ui->treeViewGroups->setHeaderHidden(true);
	ui->treeViewGroups->setAnimated(false);
	ui->treeViewGroups->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeViewGroups->setItemsExpandable(true);
	ui->treeViewGroups->setExpandsOnDoubleClick(false);
	ui->treeViewGroups->setAcceptDrops(false);
	ui->treeViewGroups->setDropIndicatorShown(false);
	ui->treeViewGroups->setAutoExpandDelay(100);
	ui->treeViewGroups->setIndentation(0);
	ui->treeViewGroups->expandAll();

	// filter line edit add completer
	ui->searchPage->addEditFilterCompleter(true, true, true, true, false);
	ui->searchPage->setTipText(searchTip);
	ui->searchPage->treeViewSearch()->setContextMenuPolicy(Qt::NoContextMenu);
	ui->searchPage->listViewSearch()->setContextMenuPolicy(Qt::NoContextMenu);

	// init to first page
	ui->stackedWidget->setCurrentIndex(0);
}

void SelectChatWidget::select(const QString &id, SelectChatWidget::Source source, const QString &extra /*= QString()*/)
{
	ModelManager *modelManager = qPmApp->getModelManager();
	switch (source)
	{
	case RosterSource:
		{
			QStandardItem *pGroup = m_selectTreeModel->rosterGroup();
			if (id.isEmpty())
			{
				onTreeViewGroupsClicked(pGroup->index());
				return;
			}

			// find roster item
			RosterModel *rosterModel = modelManager->rosterModel();
			if (!rosterModel->containsRoster(id))
				return;

			ui->treeViewGroups->setCurrentIndex(pGroup->index());
			m_selectListModel->change(source);
			QStandardItem *pRoster = m_selectListModel->getChatListItem(id);
			if (pRoster)
				onListViewMembersActivated(pRoster->index());
		}
		break;

	case OsSource:
		{
			QStandardItem *pGroup = m_selectTreeModel->osGroup();
			if (id.isEmpty())
			{
				onTreeViewGroupsClicked(pGroup->index());
				return;
			}

			// find os contact item
			OrgStructModel *orgModel = qPmApp->getModelManager()->orgStructModel();
			if (!orgModel->containsContactByUid(id))
				return;

			QString wid;
			if (extra.isEmpty())
				wid = orgModel->uid2Wid(id).value(0, "");
			else
				wid = extra;

			if (wid.isEmpty())
				return;
		
			ui->treeViewGroups->setCurrentIndex(pGroup->index());
			m_selectListModel->change(source);
			QStandardItem *pOsContact = m_selectListModel->getChatListItem(wid);
			if (pOsContact)
				onListViewMembersActivated(pOsContact->index());
		}
		break;

	case GroupSource:
		{
			QStandardItem *pGroup = m_selectTreeModel->groupGroup();
			if (id.isEmpty())
			{
				onTreeViewGroupsClicked(pGroup->index());
				return;
			}

			ui->treeViewGroups->setCurrentIndex(pGroup->index());
			m_selectListModel->change(source);
			QStandardItem *pItem = m_selectListModel->getChatListItem(id);
			if (pItem)
				onListViewMembersActivated(pItem->index());
		}
		break;

	case DiscussSource:
		{
			QStandardItem *pGroup = m_selectTreeModel->discussGroup();
			if (id.isEmpty())
			{
				onTreeViewGroupsClicked(pGroup->index());
				return;
			}

			ui->treeViewGroups->setCurrentIndex(pGroup->index());
			m_selectListModel->change(source);
			QStandardItem *pItem = m_selectListModel->getChatListItem(id);
			if (pItem)
				onListViewMembersActivated(pItem->index());
		}
		break;
	default:
		break;
	}
}

void SelectChatWidget::selectFirst()
{
	ui->treeViewGroups->collapseAll();

	QStandardItem *lastContactGroup = m_selectTreeModel->lastContactGroup();
	if (lastContactGroup) // select last contact
	{
		onTreeViewGroupsClicked(lastContactGroup->index());
		return;
	}

	QStandardItem *rosterGroup = m_selectTreeModel->rosterGroup();
	onTreeViewGroupsClicked(rosterGroup->index());
}

EditFilterWidget *SelectChatWidget::searchWidget() const
{
	return ui->searchPage;
}

void SelectChatWidget::editFilterChanged(const QString &filterText)
{
	if (filterText.isEmpty())
	{
		if (ui->stackedWidget->currentIndex() != 0)
			ui->stackedWidget->setCurrentIndex(0);
	}
	else
	{
		if (ui->stackedWidget->currentIndex() != 1)
			ui->stackedWidget->setCurrentIndex(1);
	}

	ui->searchPage->editFilterChanged(filterText);
}

void SelectChatWidget::editFilterSelected(const QString &id, int source, const QString &wid)
{
	if (ui->stackedWidget->currentIndex() != 0)
		ui->stackedWidget->setCurrentIndex(0);

	if (source == SELECT_SOURCE_ROSTER) // roster
	{
		select(id, RosterSource);

		emit itemClicked(id, RosterSource);
	}
	if (source == SELECT_SOURCE_OS) // organization structure
	{
		select(id, OsSource, wid);

		emit itemClicked(id, OsSource);
	}
	else if (source == SELECT_SOURCE_GROUP) // group
	{
		select(id, GroupSource);

		emit itemClicked(id, GroupSource);
	}
	else if (source == SELECT_SOURCE_DISCUSS) // discuss
	{
		select(id, DiscussSource);

		emit itemClicked(id, DiscussSource);
	}
}

void SelectChatWidget::onTreeViewGroupsClicked(const QModelIndex &index)
{
	ui->treeViewGroups->setCurrentIndex(index);

	int type = index.data(TypeRole).toInt();
	m_selectListModel->change((SelectChatWidget::Source)type);

	QModelIndex idx = m_selectListModel->index(0, 0);
	onListViewMembersActivated(idx);
}

void SelectChatWidget::onListViewMembersActivated(const QModelIndex &index)
{
	ui->listViewMembers->setCurrentIndex(index);
	if (!index.isValid())
	{
		emit selectChanged("", SourceInvalid);
	}
	else
	{
		int type = index.data(TypeRole).toInt();
		QString id = index.data(IDRole).toString();
		QString uid = id;
		if (OsSource == type)
		{
			OrgStructModel *orgModel = qPmApp->getModelManager()->orgStructModel();
			uid = orgModel->wid2Uid(id);
		}
		emit selectChanged(uid, type);
	}
}

void SelectChatWidget::onListViewMembersClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		int type = index.data(TypeRole).toInt();
		QString id = index.data(IDRole).toString();
		QString uid = id;
		if (OsSource == type)
		{
			OrgStructModel *orgModel = qPmApp->getModelManager()->orgStructModel();
			uid = orgModel->wid2Uid(id);
		}
		emit itemClicked(uid, type);
	}
}

#include "selectchatwidget.moc"

