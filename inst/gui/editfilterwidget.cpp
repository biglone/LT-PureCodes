#include "editfilterwidget.h"
#include "ui_editfilterwidget.h"
#include <QStandardItemModel>
#include "rosteritemdef.h"
#include "orgstructitemdef.h"
#include "groupitemdef.h"
#include "DiscussItemdef.h"
#include "rostermodeldef.h"
#include "orgstructmodeldef.h"
#include "groupmodeldef.h"
#include "DiscussModeldef.h"
#include "subscriptionmodel.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "util/PinYinConverter.h"
#include "editfiltergroupitemwidget.h"
#include "editfiltersourcedelegate.h"

const int SELECT_SOURCE_ROSTER           = 0;
const int SELECT_SOURCE_OS               = 1;
const int SELECT_SOURCE_GROUP            = 2;
const int SELECT_SOURCE_DISCUSS          = 3;
const int SELECT_SOURCE_SUBSCRIPTION     = 4;

EditFilterWidget::EditFilterWidget(QWidget *parent)
	: QWidget(parent), m_totalCount(0), m_pageIndex(0), m_pageOfCount(20), m_sourceDelegate(0)
{
	ui = new Ui::EditFilterWidget();
	ui->setupUi(this);

	setCurrentIndex(0);

	ui->labelReturn->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelPageDown->setFontAtt(QColor(0, 120, 216), 10, false);
	ui->labelPageUp->setFontAtt(QColor(0, 120, 216), 10, false);

	ui->searchDetailDownRow->setStyleSheet("QWidget#searchDetailDownRow {background: white; border: none; border-top: 1px solid rgb(220, 220, 240);}");

	ui->labelTip->setStyleSheet(QString(
		"QLabel#labelTip {"
		"	color: rgb(128, 128, 128);"
		"	background-color: rgb(231, 246, 236);"
		"	border: none;"
		"	border-bottom: 1px solid rgb(232, 232, 232);"
		"}"
		));
	m_tipText = tr("You can double click or use context menu");

	connect(ui->treeViewSearch, SIGNAL(chat(QString)), this, SIGNAL(chat(QString)));
	connect(ui->treeViewSearch, SIGNAL(groupChat(QString)), this, SIGNAL(groupChat(QString)));
	connect(ui->treeViewSearch, SIGNAL(discussChat(QString)), this, SIGNAL(discussChat(QString)));
	connect(ui->treeViewSearch, SIGNAL(subscriptionChat(QString)), this, SIGNAL(subscriptionChat(QString)));
	connect(ui->treeViewSearch, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(ui->treeViewSearch, SIGNAL(selectSearchItem(QString, int, QString)), this, SIGNAL(selectSearchItem(QString, int, QString)));
	connect(ui->treeViewSearch, SIGNAL(viewAll(int)), this, SLOT(viewAll(int)));

	connect(ui->listViewSearch, SIGNAL(chat(QString)), this, SIGNAL(chat(QString)));
	connect(ui->listViewSearch, SIGNAL(groupChat(QString)), this, SIGNAL(groupChat(QString)));
	connect(ui->listViewSearch, SIGNAL(discussChat(QString)), this, SIGNAL(discussChat(QString)));
	connect(ui->listViewSearch, SIGNAL(subscriptionChat(QString)), this, SIGNAL(subscriptionChat(QString)));
	connect(ui->listViewSearch, SIGNAL(viewMaterial(QString)), this, SIGNAL(viewMaterial(QString)));
	connect(ui->listViewSearch, SIGNAL(selectSearchItem(QString, int, QString)), this, SIGNAL(selectSearchItem(QString, int, QString)));
}

EditFilterWidget::~EditFilterWidget()
{
	delete ui;
}

EditFilterTreeView *EditFilterWidget::treeViewSearch() const
{
	return ui->treeViewSearch;
}

EditFilterListView *EditFilterWidget::listViewSearch() const
{
	return ui->listViewSearch;
}

void EditFilterWidget::setSelectMode(SelectMode mode)
{
	ui->treeViewSearch->setSelectMode(mode);
	ui->listViewSearch->setSelectMode(mode);
}

void EditFilterWidget::setCurrentIndex(int index)
{
	ui->stackedWidget->setCurrentIndex(index);
}

void EditFilterWidget::addEditFilterCompleter(bool addRoster /*= true*/, bool addOs /*= true*/, bool addGroup /*= true*/, bool addDiscuss /*= true*/, bool addSubscription /*= true*/)
{
	if (ui->treeViewSearch->editFilterModel())
	{
		return;
	}

	// create new completer, add tree view model
	QStandardItemModel *completerModel = new QStandardItemModel(this);
	ui->treeViewSearch->setEditFilterModel(completerModel);

	if (addRoster)
	{
		RosterBaseItem *rosterGroup = new RosterBaseItem(RosterBaseItem::RosterTypeGroup, QString(), tr("Friends"), 0);
		rosterGroup->setText(tr("Friends"));
		ui->treeViewSearch->setRosterGroup(rosterGroup);
	}

	if (addOs)
	{
		RosterBaseItem *osGroup = new RosterBaseItem(RosterBaseItem::RosterTypeGroup, QString(), tr("Corporation"), 0);
		osGroup->setText(tr("Corporation"));
		ui->treeViewSearch->setOsGroup(osGroup);
	}

	if (addGroup)
	{
		RosterBaseItem *mucGroup = new RosterBaseItem(RosterBaseItem::RosterTypeGroup, QString(), tr("Groups"), 0);
		mucGroup->setText(tr("Groups"));
		ui->treeViewSearch->setGroupGroup(mucGroup);
	}

	if (addDiscuss)
	{
		RosterBaseItem *discussGroup = new RosterBaseItem(RosterBaseItem::RosterTypeGroup, QString(), tr("Discusses"), 0);
		discussGroup->setText(tr("Discusses"));
		ui->treeViewSearch->setDiscussGroup(discussGroup);
	}

	if (addSubscription)
	{
		RosterBaseItem *subscriptionGroup = new RosterBaseItem(RosterBaseItem::RosterTypeGroup, QString(), ModelManager::subscriptionShowName(), 0);
		subscriptionGroup->setText(ModelManager::subscriptionShowName());
		ui->treeViewSearch->setSubscriptionGroup(subscriptionGroup);
	}

	// add list view model
	QStandardItemModel *listViewModel = new QStandardItemModel(this);
	ui->listViewSearch->setEditFilterModel(listViewModel);
}

void EditFilterWidget::editFilterChanged(const QString &filterText)
{
	QString searchString = filterText;
	if (m_searchString == searchString)
	{
		return;
	}

	m_searchString = searchString;

	setCurrentIndex(0);

	// search the right items
	int rosterCount = 0;
	int osContactCount = 0;
	int groupCount = 0;
	int discussCount = 0;
	int subscriptionCount = 0;
	int maxCount = 20;
	searchInAll(filterText, rosterCount, osContactCount, groupCount, discussCount, subscriptionCount, maxCount);

	// set the group number
	QList<int> countNum;
	countNum << rosterCount << osContactCount << groupCount << discussCount << subscriptionCount;

	RosterBaseItem *rosterGroup = ui->treeViewSearch->rosterGroup();
	RosterBaseItem *osContactGroup = ui->treeViewSearch->osContactGroup();
	RosterBaseItem *groupGroup = ui->treeViewSearch->groupGroup();
	RosterBaseItem *discussGroup = ui->treeViewSearch->discussGroup();
	RosterBaseItem *subscriptionGroup = ui->treeViewSearch->subscriptionGroup();
	QList<RosterBaseItem *> groupItems;
	groupItems << rosterGroup << osContactGroup << groupGroup << discussGroup << subscriptionGroup;
	QStringList textFormats;
	textFormats << tr("Friends") + " [%1]"
		<< tr("Corporation") + " [%1]"
		<< tr("Groups") + " [%1]" 
		<< tr("Discusses") + " [%1]"
		<< ModelManager::subscriptionShowName() + " [%1]";

	QString countText;
	QString itemText;
	bool viewAll = false;
	for (int i = 0; i < groupItems.count(); i++)
	{
		int num = countNum[i];
		if (num >= maxCount)
		{
			viewAll = true;
			countText = QString("%1+").arg(maxCount);
		}
		else
		{
			viewAll = false;
			countText = QString::number(num);
		}
		itemText = textFormats[i].arg(countText);

		if (i == 0)
		{
			ui->treeViewSearch->setRosterGroupInfo(itemText, viewAll);
		}
		else if (i == 1)
		{
			ui->treeViewSearch->setOsGroupInfo(itemText, viewAll);
		}
		else if (i == 2)
		{
			ui->treeViewSearch->setGroupGroupInfo(itemText, viewAll);
		}
		else if (i == 3)
		{
			ui->treeViewSearch->setDiscussGroupInfo(itemText, viewAll);
		}
		else if (i == 4)
		{
			ui->treeViewSearch->setSubscriptionGroupInfo(itemText, viewAll);
		}
	}

	// update header data
	int searchCount = rosterCount + osContactCount + groupCount + discussCount + subscriptionCount;
	if (searchCount > 0)
	{
		ui->labelTip->setText(m_tipText);
	}
	else
	{
		ui->labelTip->setText(tr("No search results..."));
	}

	ui->treeViewSearch->setExpandAll();

	ui->treeViewSearch->selectFirstItem();
}

void EditFilterWidget::setTipText(const QString &tipText)
{
	m_tipText = tipText;
}

QString EditFilterWidget::tipText() const
{
	return m_tipText;
}

void EditFilterWidget::setTipTextVisible(bool visible)
{
	ui->labelTip->setVisible(visible);
}

void EditFilterWidget::setSourceDelegate(EditFilterSourceDelegate *sourceDelegate)
{
	m_sourceDelegate = sourceDelegate;
}

void EditFilterWidget::viewAll(int type)
{
	m_pageIndex = 0;
	m_totalCount = 0;

	if (type == EditFilterGroupItemWidget::Roster)
	{
		searchRoster(m_searchString);
	}
	else if (type == EditFilterGroupItemWidget::Os)
	{
		searchOs(m_searchString);
	}
	else if (type == EditFilterGroupItemWidget::Group)
	{
		searchGroup(m_searchString);
	}
	else if (type == EditFilterGroupItemWidget::Discuss)
	{
		searchDiscuss(m_searchString);
	}
	else if (type == EditFilterGroupItemWidget::Subscription)
	{
		searchSubscription(m_searchString);
	}

	onSearchFinished();

	setCurrentIndex(1);
}

void EditFilterWidget::on_labelReturn_clicked()
{
	setCurrentIndex(0);
}

void EditFilterWidget::on_labelPageUp_clicked()
{
	--m_pageIndex;
	if (m_pageIndex < 0)
	{
		m_pageIndex = 0;
	}
	
	if (ui->listViewSearch->listType() == EditFilterListView::Roster)
	{
		searchRoster(m_searchString);
	}
	else if (ui->listViewSearch->listType() == EditFilterListView::Os)
	{
		searchOs(m_searchString);
	}
	else if (ui->listViewSearch->listType() == EditFilterListView::Group)
	{
		searchGroup(m_searchString);
	}
	else if (ui->listViewSearch->listType() == EditFilterListView::Discuss)
	{
		searchDiscuss(m_searchString);
	}

	onSearchFinished();
}

void EditFilterWidget::on_labelPageDown_clicked()
{
	++m_pageIndex;
	int totalPage = m_totalCount/m_pageOfCount;
	if (m_totalCount%m_pageOfCount)
	{
		++totalPage;
	}
	if (m_pageIndex >= totalPage)
	{
		m_pageIndex = totalPage - 1;
	}

	if (ui->listViewSearch->listType() == EditFilterListView::Roster)
	{
		searchRoster(m_searchString);
	}
	else if (ui->listViewSearch->listType() == EditFilterListView::Os)
	{
		searchOs(m_searchString);
	}
	else if (ui->listViewSearch->listType() == EditFilterListView::Group)
	{
		searchGroup(m_searchString);
	}
	else if (ui->listViewSearch->listType() == EditFilterListView::Discuss)
	{
		searchDiscuss(m_searchString);
	}

	onSearchFinished();
}

void EditFilterWidget::searchInAll(const QString &searchStr, 
	                               int &rosterCount, 
								   int &osCount, 
								   int &groupCount, 
								   int &discussCount,
								   int &subscriptionCount,
	                               int maxCount /*= 100*/)
{
	rosterCount = 0;
	osCount = 0;
	groupCount = 0;
	discussCount = 0;
	subscriptionCount = 0;

	// clear all the select items
	ui->treeViewSearch->clearAll();

	if (searchStr.isEmpty())
	{
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	OrgStructModel *osModel = modelManager->orgStructModel();
	GroupModel *groupModel = modelManager->groupModel();
	DiscussModel *discussModel = modelManager->discussModel();
	SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();

	RosterBaseItem *rosterGroup = ui->treeViewSearch->rosterGroup();
	RosterBaseItem *osContactGroup = ui->treeViewSearch->osContactGroup();
	RosterBaseItem *groupGroup = ui->treeViewSearch->groupGroup();
	RosterBaseItem *discussGroup = ui->treeViewSearch->discussGroup();
	RosterBaseItem *subscriptionGroup = ui->treeViewSearch->subscriptionGroup();

	QString id;
	QString name;
	QString filterName;
	QString groupName;
	QString matchStr;

	if (rosterGroup)
	{
		// find the right roster
		QStringList allRosterIds = getRosterIds();
		foreach (id, allRosterIds)
		{
			name = rosterModel->rosterName(id);
			if (getMatchName(name, searchStr, matchStr))
			{
				RosterBaseItem *item = new RosterBaseItem(RosterBaseItem::RosterTypeContact, id, name, 0);
				filterName = name;
				item->setText(filterName);
				item->setToolTip(filterName);
				item->setExtraInfo2(matchStr);
				rosterGroup->appendRow(item);

				if (++rosterCount >= maxCount)
				{
					break;
				}
			}
		}
	}

	if (osContactGroup)
	{
		// find the right os contact
		QStringList allContactWids = getOsWids();
		foreach (QString wid, allContactWids)
		{
			OrgStructContactItem *contact = osModel->contactByWid(wid);
			name = contact->itemName();
			id = osModel->wid2Uid(wid);
			if (getMatchName(name, searchStr, matchStr))
			{
				RosterBaseItem *item = new OrgStructContactItem(wid, id, name, 0);
				filterName = name;
				item->setText(filterName);
				item->setToolTip(filterName);
				QString deptName = osModel->contactGroupNameByWid(wid);
				item->setExtraInfo(deptName);
				item->setExtraInfo2(matchStr);
				osContactGroup->appendRow(item);

				if (++osCount >= maxCount)
				{
					break;
				}
			}
		}
	}

	if (groupGroup)
	{
		// find the right group
		QStringList allGroupIds = getGroupIds();
		foreach (id, allGroupIds)
		{
			MucGroupItem *groupItem = groupModel->getGroup(id);
			if (groupItem)
			{
				name = groupItem->itemName();
				if (getMatchName(name, searchStr, matchStr))
				{
					RosterBaseItem *item = new RosterBaseItem(groupItem->itemType(), id, name, groupItem->itemIndex());
					filterName = groupItem->itemName();
					item->setText(filterName);
					item->setToolTip(filterName);
					item->setExtraInfo(QString());
					item->setExtraInfo2(matchStr);
					groupGroup->appendRow(item);

					if (++groupCount >= maxCount)
					{
						break;
					}
				}
			}
		}
	}

	if (discussGroup)
	{
		// find the right discuss
		QStringList allDiscussIds = getDiscussIds();
		foreach (id, allDiscussIds)
		{
			DiscussItem *discussItem = discussModel->getDiscuss(id);
			if (discussItem)
			{
				name = discussItem->itemName();
				if (getMatchName(name, searchStr, matchStr))
				{
					RosterBaseItem *item = new RosterBaseItem(discussItem->itemType(), id, name, discussItem->itemIndex());
					filterName = discussItem->itemName();
					item->setText(filterName);
					item->setToolTip(filterName);
					item->setExtraInfo(QString());
					item->setExtraInfo2(matchStr);
					discussGroup->appendRow(item);

					if (++discussCount >= maxCount)
					{
						break;
					}
				}
			}
		}
	}

	if (subscriptionGroup)
	{
		// find the right subscription
		QStringList allSubscriptionIds = getSubscriptionIds();
		foreach (id, allSubscriptionIds)
		{
			SubscriptionDetail subscription = subscriptionModel->subscription(id);
			if (subscription.isValid())
			{
				name = subscription.name();
				if (getMatchName(name, searchStr, matchStr))
				{
					RosterBaseItem *item = new RosterBaseItem(RosterBaseItem::RosterTypeSubscription, id, name, 0);
					filterName = name;
					item->setText(filterName);
					item->setToolTip(filterName);
					item->setExtraInfo(QString());
					item->setExtraInfo2(matchStr);
					subscriptionGroup->appendRow(item);

					if (++subscriptionCount >= maxCount)
					{
						break;
					}
				}
			}
		}
	}
}

bool EditFilterWidget::getMatchName(const QString &name, const QString &searchStr, QString &matchPart, const QString &id /*= QString()*/)
{
	bool ret = false;
	matchPart.clear();

	ModelManager *modelManager = qPmApp->getModelManager();
	const QHash<QString, QStringList> &allNamePinyin = modelManager->allNamePinyin();
	QStringList qp;
	if (allNamePinyin.contains(name))
	{
		qp = allNamePinyin[name];
	}
	else
	{
		qp = PinyinConveter::instance().quanpin(name);
	}
	QStringList firstCh = PinyinConveter::instance().firstCharsFromQuanpin(qp);

	if (qp.join("").startsWith(searchStr, Qt::CaseInsensitive))
	{
		ret = true;

		int end = 0;
		QString str = searchStr;
		while (!str.isEmpty())
		{
			if (str.length() <= qp[end].length())
				break;

			str = str.mid(qp[end].length());
			++end;
		}
		matchPart = name.left(end+1);
	}
	else if (firstCh.join("").startsWith(searchStr, Qt::CaseInsensitive))
	{
		ret = true;

		int end = 0;
		QString str = searchStr;
		while (!str.isEmpty())
		{
			if (str.length() <= firstCh[end].length())
				break;

			str = str.mid(firstCh[end].length());
			++end;
		}
		matchPart = name.left(end+1);
	}
	else if (name.contains(searchStr, Qt::CaseInsensitive))
	{
		ret = true;

		matchPart = searchStr;
	}
	else if (!id.isEmpty() && id.startsWith(searchStr, Qt::CaseInsensitive))
	{
		ret = true;

		matchPart = searchStr;
	}
	else
	{
		ret = false;
	}

	return ret;
}

void EditFilterWidget::searchRoster(const QString &searchStr)
{
	// clear all the select items
	ui->listViewSearch->clearAll();

	if (searchStr.isEmpty())
	{
		return;
	}

	ui->listViewSearch->setListType(EditFilterListView::Roster);

	ModelManager *modelManager = qPmApp->getModelManager();
	RosterModel *rosterModel = modelManager->rosterModel();
	
	QString id;
	QString name;
	QString filterName;
	QString groupName;
	QString matchStr;
	int startIndex = m_pageIndex*m_pageOfCount;
	int endIndex = startIndex + m_pageOfCount;
	m_totalCount = 0;

	// find the right roster
	QStringList allRosterIds = getRosterIds();
	foreach (id, allRosterIds)
	{
		name = rosterModel->rosterName(id);
		if (getMatchName(name, searchStr, matchStr))
		{
			if (m_totalCount >= startIndex && m_totalCount < endIndex)
			{
				RosterBaseItem *item = new RosterBaseItem(RosterBaseItem::RosterTypeContact, id, name, 0);
				filterName = name;
				item->setText(filterName);
				item->setToolTip(filterName);
				item->setExtraInfo2(matchStr);
				ui->listViewSearch->addItem(item);
			}

			++m_totalCount;
		}
	}
}

void EditFilterWidget::searchOs(const QString &searchStr)
{
	// clear all the select items
	ui->listViewSearch->clearAll();

	if (searchStr.isEmpty())
	{
		return;
	}

	ui->listViewSearch->setListType(EditFilterListView::Os);

	ModelManager *modelManager = qPmApp->getModelManager();
	OrgStructModel *osModel = modelManager->orgStructModel();

	QString id;
	QString name;
	QString filterName;
	QString groupName;
	QString matchStr;
	int startIndex = m_pageIndex*m_pageOfCount;
	int endIndex = startIndex + m_pageOfCount;
	m_totalCount = 0;

	// find the right os contact
	QStringList allContactWids = getOsWids();
	foreach (QString wid, allContactWids)
	{
		OrgStructContactItem *contact = osModel->contactByWid(wid);
		name = contact->itemName();
		id = osModel->wid2Uid(wid);
		if (getMatchName(name, searchStr, matchStr))
		{
			if (m_totalCount >= startIndex && m_totalCount < endIndex)
			{
				RosterBaseItem *item = new OrgStructContactItem(wid, id, name, 0);
				filterName = name;
				item->setText(filterName);
				item->setToolTip(filterName);
				QString deptName = osModel->contactGroupNameByWid(wid);
				item->setExtraInfo(deptName);
				item->setExtraInfo2(matchStr);
				ui->listViewSearch->addItem(item);
			}

			++m_totalCount;
		}
	}
}

void EditFilterWidget::searchGroup(const QString &searchStr)
{
	// clear all the select items
	ui->listViewSearch->clearAll();

	if (searchStr.isEmpty())
	{
		return;
	}

	ui->listViewSearch->setListType(EditFilterListView::Group);

	ModelManager *modelManager = qPmApp->getModelManager();
	GroupModel *groupModel = modelManager->groupModel();

	QString id;
	QString name;
	QString filterName;
	QString groupName;
	QString matchStr;
	int startIndex = m_pageIndex*m_pageOfCount;
	int endIndex = startIndex + m_pageOfCount;
	m_totalCount = 0;

	// find the right group
	QStringList allGroupIds = getGroupIds();
	foreach (id, allGroupIds)
	{
		MucGroupItem *groupItem = groupModel->getGroup(id);
		if (groupItem)
		{
			name = groupItem->itemName();
			if (getMatchName(name, searchStr, matchStr))
			{
				if (m_totalCount >= startIndex && m_totalCount < endIndex)
				{
					RosterBaseItem *item = new RosterBaseItem(groupItem->itemType(), id, name, groupItem->itemIndex());
					filterName = groupItem->itemName();
					item->setText(filterName);
					item->setToolTip(filterName);
					item->setExtraInfo(QString());
					item->setExtraInfo2(matchStr);
					ui->listViewSearch->addItem(item);
				}

				++m_totalCount;
			}
		}
	}
}

void EditFilterWidget::searchDiscuss(const QString &searchStr)
{
	// clear all the select items
	ui->listViewSearch->clearAll();

	if (searchStr.isEmpty())
	{
		return;
	}

	ui->listViewSearch->setListType(EditFilterListView::Discuss);

	ModelManager *modelManager = qPmApp->getModelManager();
	DiscussModel *discussModel = modelManager->discussModel();

	QString id;
	QString name;
	QString filterName;
	QString groupName;
	QString matchStr;
	int startIndex = m_pageIndex*m_pageOfCount;
	int endIndex = startIndex + m_pageOfCount;
	m_totalCount = 0;

	// find the right discuss
	QStringList allDiscussIds = getDiscussIds();
	foreach (id, allDiscussIds)
	{
		DiscussItem *discussItem = discussModel->getDiscuss(id);
		if (discussItem)
		{
			name = discussItem->itemName();
			if (getMatchName(name, searchStr, matchStr))
			{
				if (m_totalCount >= startIndex && m_totalCount < endIndex)
				{
					RosterBaseItem *item = new RosterBaseItem(discussItem->itemType(), id, name, discussItem->itemIndex());
					filterName = discussItem->itemName();
					item->setText(filterName);
					item->setToolTip(filterName);
					item->setExtraInfo(QString());
					item->setExtraInfo2(matchStr);
					ui->listViewSearch->addItem(item);
				}
				
				++m_totalCount;
			}
		}
	}
}

void EditFilterWidget::searchSubscription(const QString &searchStr)
{
	// clear all the select items
	ui->listViewSearch->clearAll();

	if (searchStr.isEmpty())
	{
		return;
	}

	ui->listViewSearch->setListType(EditFilterListView::Subscription);

	ModelManager *modelManager = qPmApp->getModelManager();
	SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();

	QString id;
	QString name;
	QString filterName;
	QString groupName;
	QString matchStr;
	int startIndex = m_pageIndex*m_pageOfCount;
	int endIndex = startIndex + m_pageOfCount;
	m_totalCount = 0;

	// find the right subscription
	QStringList allSubscriptionIds = getSubscriptionIds();
	foreach (id, allSubscriptionIds)
	{
		SubscriptionDetail subscription = subscriptionModel->subscription(id);
		if (subscription.isValid())
		{
			name = subscription.name();
			if (getMatchName(name, searchStr, matchStr))
			{
				if (m_totalCount >= startIndex && m_totalCount < endIndex)
				{
					RosterBaseItem *item = new RosterBaseItem(RosterBaseItem::RosterTypeSubscription, id, name, 0);
					filterName = name;
					item->setText(filterName);
					item->setToolTip(filterName);
					item->setExtraInfo(QString());
					item->setExtraInfo2(matchStr);
					ui->listViewSearch->addItem(item);
				}

				++m_totalCount;
			}
		}
	}
}

void EditFilterWidget::onSearchFinished()
{
	ui->labelPageCount->setText(tr("Total %1").arg(m_totalCount));

	int totalPage = m_totalCount/m_pageOfCount;
	if (m_totalCount%m_pageOfCount)
	{
		++totalPage;
	}
	int currentPage = m_pageIndex + 1;
	ui->labelPageAll->setText(tr("%1/%2").arg(currentPage).arg(totalPage));

	if (m_pageIndex > 0)
	{
		ui->labelPageUp->setEnabled(true);
	}
	else
	{
		ui->labelPageUp->setEnabled(false);
	}

	if ((m_pageIndex+1)*m_pageOfCount < m_totalCount)
	{
		ui->labelPageDown->setEnabled(true);
	}
	else
	{
		ui->labelPageDown->setEnabled(false);
	}
}

QStringList EditFilterWidget::getRosterIds()
{
	QStringList rosterIds;
	if (m_sourceDelegate)
	{
		rosterIds = m_sourceDelegate->getRosterIds();
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		RosterModel *rosterModel = modelManager->rosterModel();
		rosterIds = rosterModel->allRosterIds();
	}
	return rosterIds;
}

QStringList EditFilterWidget::getOsWids()
{
	QStringList osWids;
	if (m_sourceDelegate)
	{
		osWids = m_sourceDelegate->getOsWids();
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		OrgStructModel *osModel = modelManager->orgStructModel();
		osWids = osModel->allContactWids();
	}
	return osWids;
}

QStringList EditFilterWidget::getGroupIds()
{
	QStringList groupIds;
	if (m_sourceDelegate)
	{
		groupIds = m_sourceDelegate->getGroupIds();
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		GroupModel *groupModel = modelManager->groupModel();
		groupIds = groupModel->allGroupIds();
	}
	return groupIds;
}

QStringList EditFilterWidget::getDiscussIds()
{
	QStringList discussIds;
	if (m_sourceDelegate)
	{
		discussIds = m_sourceDelegate->getDiscussIds();
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		DiscussModel *discussModel = modelManager->discussModel();
		discussIds = discussModel->allDiscussIds();
	}
	return discussIds;
}

QStringList EditFilterWidget::getSubscriptionIds()
{
	QStringList subscriptionIds;
	if (m_sourceDelegate)
	{
		subscriptionIds = m_sourceDelegate->getSubscriptionIds();
	}
	else
	{
		ModelManager *modelManager = qPmApp->getModelManager();
		SubscriptionModel *subscriptionModel = modelManager->subscriptionModel();
		subscriptionIds = subscriptionModel->allSubscriptionIds();
	}
	return subscriptionIds;
}