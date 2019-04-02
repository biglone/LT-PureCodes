#include "discusstreeview.h"
#include "discussitemdelegate.h"
#include "model/discussmodeldef.h"
#include <QEvent>
#include <QStandardItem>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QUrl>
#include "model/sortfilterproxymodel.h"
#include "model/discussitemdef.h"
#include "flickerhelper.h"
#include "PmApp.h"
#include "Account.h"
#include <QPainter>
#include "ModelManager.h"
#include "configmanager.h"
#include "iospushmanager.h"

DiscussTreeView::DiscussTreeView(QWidget *parent) : QTreeView(parent)
{
	m_itemDelegate = new DiscussItemDelegate(this);
	setItemDelegate(m_itemDelegate);

	setRootIsDecorated(false);
	setAnimated(false);
	setItemsExpandable(false);
	setExpandsOnDoubleClick(false);
	setIndentation(0);
	setHeaderHidden(true);

	setContextMenuPolicy(Qt::CustomContextMenu);
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setEditTriggers(NoEditTriggers);

	setDragEnabled(false);
	setAcceptDrops(false);
	setDropIndicatorShown(false);

	setupActions();
	setSkin();

	connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked(const QModelIndex&)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(const QPoint&)));
}

void DiscussTreeView::setDiscussModel(DiscussModel *discussModel)
{
	m_discussModel = discussModel;
	m_itemDelegate->setDiscussModel(discussModel);
	if (m_discussModel->proxyModel())
		setModel(m_discussModel->proxyModel());
	else
		setModel(m_discussModel);
	expandAll();
	connect(m_discussModel, SIGNAL(discussAdded(QString)), this, SLOT(onDiscussAdded(QString)));
}

void DiscussTreeView::setFlickerHelper(FlickerHelper *flickerHelper)
{
	m_itemDelegate->setFlickerHelper(flickerHelper);
}

bool DiscussTreeView::containsFlickerItem(const QString &id, bean::MessageType msgType) const
{
	if (!m_discussModel)
		return false;

	if (msgType != bean::Message_DiscussChat)
		return false;

	return m_discussModel->containsDiscuss(id);
}

void DiscussTreeView::doUpdate()
{
	update();
}

void DiscussTreeView::doubleClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		DiscussItem *discussItem = m_discussModel->nodeFromProxyIndex(index);
		if (discussItem)
		{
			emit chat(discussItem->itemId());
		}
	}
}

void DiscussTreeView::groupSetting(QAction *action)
{
	if (!action)
		return;
	QString id = action->data().toString();
	AccountSettings::GroupMsgSettingType setting = Account::settings()->discussMsgSetting(id);
	bool tipChanged = false;
	int silence = 0;
	if (action == m_groupTip)
	{
		if (AccountSettings::UnTip== setting)
		{
			tipChanged = true;
		}
		setting = AccountSettings::Tip;
	}
	else
	{
		if (AccountSettings::Tip == setting)
		{
			tipChanged = true;
		}
		setting = AccountSettings::UnTip;
		silence = 1;
	}

	if (tipChanged)
	{
		// set discuss message setting
		Account::settings()->setDiscussMsgSetting(id, setting);

		// set conf3
		QStringList silenceList = Account::settings()->silenceList();
		ConfigManager *configManager = qPmApp->getConfigManager();
		configManager->setConfig3(silenceList);

		// ios push
		IOSPushManager *pIOSPushManager = qPmApp->getIOSPushManager();
		pIOSPushManager->pushForIOS("discuss", id, silence);
	}
}

void DiscussTreeView::changeName()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit changeName(id);
}

void DiscussTreeView::onDiscussAdded(const QString &id)
{
	DiscussItem *discuss = m_discussModel->getDiscuss(id);
	if (discuss)
	{
		QStandardItem *group = discuss->parent();
		if (group)
		{
			QModelIndex groupIndex = group->index();
			if (m_discussModel->proxyModel())
				groupIndex = m_discussModel->proxyModel()->mapFromSource(group->index());
			if (!isExpanded(groupIndex))
				expand(groupIndex);
		}
	}
}

void DiscussTreeView::paintEvent(QPaintEvent *event)
{
	if (m_discussModel && m_discussModel->rowCount() > 0)
		return QTreeView::paintEvent(event);

	QPainter painter(this->viewport());
	QRect rt = rect();
	rt.setHeight(rt.height()/2);
	QFont originalFont = painter.font();
	QFont smallFont = originalFont;
	smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/5);
	painter.setFont(smallFont);
	painter.setPen(QColor(160, 160, 160));
	painter.setBrush(Qt::NoBrush);
	painter.drawText(rt, Qt::AlignHCenter|Qt::AlignVCenter, tr("No discusses"));
	painter.setFont(originalFont);
}

DiscussItem *DiscussTreeView::selectedItem()
{
	if(selectedIndexes().size() > 0 && selectedIndexes().at(0).isValid())
	{
		return m_discussModel->nodeFromProxyIndex(selectedIndexes().at(0));
	}
	else
	{
		return 0;
	}
}


void DiscussTreeView::chat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit chat(id);
}

void DiscussTreeView::viewPastChat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewPastChat(id);
}

void DiscussTreeView::exitDiscuss()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit exitDiscuss(id);
}

void DiscussTreeView::disbandDiscuss()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit disbandDiscuss(id);
}

void DiscussTreeView::contextMenu(const QPoint &position)
{
	QModelIndex index = indexAt(position);
	if (index.isValid())
	{
		// set action data
		DiscussItem *discussItem = m_discussModel->nodeFromProxyIndex(index);
		if (!discussItem)
			return;

		QString id = discussItem->itemId();

		if (discussItem->creator() == Account::instance()->id())
		{
			m_exitDiscussAction->setText(tr("Disband Discuss"));
			disconnect(m_exitDiscussAction, 0, this, 0);
			connect(m_exitDiscussAction, SIGNAL(triggered()), SLOT(disbandDiscuss()));
		}
		else
		{
			m_exitDiscussAction->setText(tr("Quit Discuss"));
			disconnect(m_exitDiscussAction, 0, this, 0);
			connect(m_exitDiscussAction, SIGNAL(triggered()), SLOT(exitDiscuss()));
		}

		m_chatAction->setData(id);
		m_viewPastChatsAction->setData(id);
		m_groupTip->setData(id);
		m_groupUntip->setData(id);
		m_exitDiscussAction->setData(id);
		m_changeNameAction->setData(id);

		// show menu
		QMenu menu(this);
		menu.addAction(m_chatAction);
		menu.setDefaultAction(m_chatAction);
		// menu.addAction(m_multiMailAction);
		menu.addSeparator();
		menu.addAction(m_changeNameAction);
		menu.addAction(m_viewPastChatsAction);

		QMenu *msgSettingMenu = menu.addMenu(tr("Message setting of discuss"));
		msgSettingMenu->addAction(m_groupTip);
		msgSettingMenu->addAction(m_groupUntip);

		AccountSettings::GroupMsgSettingType setting = Account::settings()->discussMsgSetting(id);
		if (setting == AccountSettings::Tip)
			m_groupTip->setChecked(true);
		else
			m_groupUntip->setChecked(true);

		menu.addAction(m_exitDiscussAction);

		menu.exec(QCursor::pos());
	}
}

void DiscussTreeView::setupActions()
{
	m_chatAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chatAction, SIGNAL(triggered()), SLOT(chat()));

	m_multiMailAction = new QAction(/*QIcon(":/images/Icon_103.png"),*/ tr("Mail to Others"), this);
	connect(m_multiMailAction, SIGNAL(triggered()), SIGNAL(multiMail()));

	m_viewPastChatsAction = new QAction(QIcon(":/images/Icon_102.png"), tr("View Chat History"), this);
	connect(m_viewPastChatsAction, SIGNAL(triggered()), SLOT(viewPastChat()));

	m_exitDiscussAction = new QAction(tr("Quit Discuss"), this);
	connect(m_exitDiscussAction, SIGNAL(triggered()), SLOT(exitDiscuss()));

	m_changeNameAction = new QAction(tr("Change name of discuss"), this);
	connect(m_changeNameAction, SIGNAL(triggered()), SLOT(changeName()));

	m_groupActionGroup = new QActionGroup(this);
	m_groupTip = new QAction(tr("Tip normal"), m_groupActionGroup);
	m_groupTip->setCheckable(true);
	m_groupUntip = new QAction(tr("Not tip"), m_groupActionGroup);
	m_groupUntip->setCheckable(true);
	m_groupActionGroup->addAction(m_groupTip);
	m_groupActionGroup->addAction(m_groupUntip);
	m_groupActionGroup->setExclusive(true);
	m_groupTip->setChecked(true);
	connect(m_groupActionGroup, SIGNAL(triggered(QAction *)), SLOT(groupSetting(QAction *)));
}

void DiscussTreeView::setSkin()
{
	setStyleSheet("QTreeView{"
		"border: none;"
		"background-color: transparent;"
		"font-size: 10.5pt;"
		"}"
		);
}
