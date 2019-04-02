#include "grouplistview.h"
#include "groupitemdelegate.h"
#include "model/groupmodeldef.h"
#include <QEvent>
#include <QStandardItem>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QUrl>
#include "model/sortfilterproxymodel.h"
#include "model/groupitemdef.h"
#include "flickerhelper.h"
#include "PmApp.h"
#include "Account.h"
#include <QPainter>
#include "ModelManager.h"
#include "configmanager.h"
#include "iospushmanager.h"

GroupListView::GroupListView(QWidget *parent) : QListView(parent)
{
	m_itemDelegate = new GroupItemDelegate(this);
	setItemDelegate(m_itemDelegate);

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

void GroupListView::setGroupModel(GroupModel *groupModel)
{
	m_groupModel = groupModel;
	m_itemDelegate->setGroupModel(groupModel);
	if (m_groupModel->proxyModel())
		setModel(m_groupModel->proxyModel());
	else
		setModel(m_groupModel);
}

void GroupListView::setFlickerHelper(FlickerHelper *flickerHelper)
{
	m_itemDelegate->setFlickerHelper(flickerHelper);
}

bool GroupListView::containsFlickerItem(const QString &id, bean::MessageType msgType) const
{
	if (!m_groupModel)
		return false;

	if (msgType != bean::Message_GroupChat)
		return false;

	return m_groupModel->containsGroup(id);
}

void GroupListView::doUpdate()
{
	update();
}

void GroupListView::doubleClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		MucGroupItem *groupItem = m_groupModel->nodeFromProxyIndex(index);
		if (groupItem)
		{
			emit chat(groupItem->itemId());
		}
	}
}

void GroupListView::groupSetting(QAction *action)
{
	if (!action)
		return;
	QString id = action->data().toString();
	bool tipChanged = false;
	int silence = 0;
	AccountSettings::GroupMsgSettingType setting = Account::settings()->groupMsgSetting(id);
	if (action == m_groupTip)
	{
		if (AccountSettings::UnTip == setting)
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
		// set group message setting
		Account::settings()->setGroupMsgSetting(id, setting);

		// set conf3
		QStringList silenceList = Account::settings()->silenceList();
		ConfigManager *configManager = qPmApp->getConfigManager();
		configManager->setConfig3(silenceList);

		// ios push
		IOSPushManager *pIOSPushManager = qPmApp->getIOSPushManager();
		pIOSPushManager->pushForIOS("group", id, silence);
	}
}

void GroupListView::paintEvent(QPaintEvent *event)
{
	if (m_groupModel && m_groupModel->rowCount() > 0)
		return QListView::paintEvent(event);

	QPainter painter(this->viewport());
	QRect rt = rect();
	rt.setHeight(rt.height()/2);
	QFont originalFont = painter.font();
	QFont smallFont = originalFont;
	smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/5);
	painter.setFont(smallFont);
	painter.setPen(QColor(160, 160, 160));
	painter.setBrush(Qt::NoBrush);
	painter.drawText(rt, Qt::AlignHCenter|Qt::AlignVCenter, tr("No groups"));
	painter.setFont(originalFont);
}

MucGroupItem *GroupListView::selectedItem()
{
	if(selectedIndexes().size() > 0 && selectedIndexes().at(0).isValid())
	{
		return m_groupModel->nodeFromProxyIndex(selectedIndexes().at(0));
	}
	else
	{
		return 0;
	}
}


void GroupListView::chat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit chat(id);
}

void GroupListView::viewPastChat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewPastChat(id);
}

void GroupListView::sendFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit sendFile(id, QStringList());
}

void GroupListView::contextMenu(const QPoint &position)
{
	QModelIndex index = indexAt(position);
	if (index.isValid())
	{
		// set action data
		MucGroupItem *groupItem = m_groupModel->nodeFromProxyIndex(index);
		if (!groupItem)
			return;

		QString id = groupItem->itemId();
		m_chatAction->setData(id);
		// m_sendFileAction->setData(id);
		m_viewPastChatsAction->setData(id);
		m_groupTip->setData(id);
		m_groupUntip->setData(id);

		// show menu
		QMenu menu(this);
		menu.addAction(m_chatAction);
		menu.setDefaultAction(m_chatAction);
		// menu.addAction(m_multiMailAction);
		// menu.addAction(m_sendFileAction);
		menu.addSeparator();
		menu.addAction(m_viewPastChatsAction);

		QMenu *msgSettingMenu = menu.addMenu(tr("Group Message Setting"));
		msgSettingMenu->addAction(m_groupTip);
		msgSettingMenu->addAction(m_groupUntip);

		AccountSettings::GroupMsgSettingType setting = Account::settings()->groupMsgSetting(id);
		if (setting == AccountSettings::Tip)
			m_groupTip->setChecked(true);
		else
			m_groupUntip->setChecked(true);

		menu.exec(QCursor::pos());
	}
}

void GroupListView::setupActions()
{
	m_chatAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chatAction, SIGNAL(triggered()), SLOT(chat()));

	m_multiMailAction = new QAction(/*QIcon(":/images/Icon_103.png"),*/ tr("Mail to Others"), this);
	connect(m_multiMailAction, SIGNAL(triggered()), SIGNAL(multiMail()));

	m_sendFileAction = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Send File"), this);
	connect(m_sendFileAction, SIGNAL(triggered()), SLOT(sendFile()));

	m_viewPastChatsAction = new QAction(QIcon(":/images/Icon_102.png"), tr("View Chat History"), this);
	connect(m_viewPastChatsAction, SIGNAL(triggered()), SLOT(viewPastChat()));

	m_groupActionGroup = new QActionGroup(this);
	m_groupTip = new QAction(tr("Tip normal"), m_groupActionGroup);
	m_groupTip->setCheckable(true);
	m_groupUntip = new QAction(tr("No tip"), m_groupActionGroup);
	m_groupUntip->setCheckable(true);
	m_groupActionGroup->addAction(m_groupTip);
	m_groupActionGroup->addAction(m_groupUntip);
	m_groupActionGroup->setExclusive(true);
	m_groupTip->setChecked(true);
	connect(m_groupActionGroup, SIGNAL(triggered(QAction *)), SLOT(groupSetting(QAction *)));
}

void GroupListView::setSkin()
{
	setStyleSheet("QListView{"
		"border: none;"
		"background-color: transparent;"
		"font-size: 10.5pt;"
		"}"
	);
}
