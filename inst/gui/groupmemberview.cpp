#include "groupmemberview.h"
#include "groupmemberitemdelegate.h"
#include "model/groupitemlistmodeldef.h"
#include <QEvent>
#include <QStandardItem>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QUrl>
#include "model/sortfilterproxymodel.h"
#include "model/rosteritemdef.h"
#include "Account.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "model/rostermodeldef.h"
#include "plaintextlineinput.h"
#include "pmessagebox.h"

GroupMemberView::GroupMemberView(QWidget *parent)
: QListView(parent), m_groupMemberModel(0), m_itemDelegate(0), m_menuDelegate(0)
{
	m_itemDelegate = new GroupMemberItemDelegate(this);
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

void GroupMemberView::setGroupMemberModel(GroupItemListModel *groupMemberModel)
{
	m_groupMemberModel = groupMemberModel;
	m_itemDelegate->setGroupMemberModel(groupMemberModel);
	if (m_groupMemberModel->proxyModel())
		setModel(m_groupMemberModel->proxyModel());
	else
		setModel(m_groupMemberModel);
}

GroupItemListModel *GroupMemberView::groupMemberModel() const
{
	return m_groupMemberModel;
}

void GroupMemberView::setMenuDelegate(GroupMemberViewMenuDelegate *menuDelegate)
{
	m_menuDelegate = menuDelegate;
}

void GroupMemberView::doubleClicked(const QModelIndex &index)
{
	if (index.isValid())
	{
		RosterContactItem *rosterItem = m_groupMemberModel->nodeFromProxyIndex(index);
		if (rosterItem)
		{
			QString id = rosterItem->itemId();
			if (id == Account::instance()->id()) // check if is self
			{
				emit viewMaterial(id);
			}
			else
			{
				emit chat(id);
			}
		}
	}
}

void GroupMemberView::contextMenu(const QPoint &position)
{
	QModelIndex index = indexAt(position);
	if (index.isValid())
	{
		RosterContactItem *rosterItem = m_groupMemberModel->nodeFromProxyIndex(index);
		if (rosterItem)
		{
			QString id = rosterItem->itemId();

			// set action data here
			m_chat->setData(id);
			m_sendMail->setData(id);
			m_viewMaterial->setData(id);
			m_addFriend->setData(id);
			m_at->setData(id);
			m_changeCardName->setData(id);
			if (qPmApp->GetLoginMgr()->isLogined())
				m_changeCardName->setEnabled(true);
			else
				m_changeCardName->setEnabled(false);

			QMenu menu(this);

			// add all menu actions
			if (id != Account::instance()->id())
			{
				menu.addAction(m_chat);
				menu.setDefaultAction(m_chat);
				menu.addAction(m_at);
				menu.addAction(m_sendMail);
			}
			else
			{
				menu.addAction(m_sendMail);
			}

			menu.addSeparator();
			menu.addAction(m_viewMaterial);

			if (id != Account::instance()->id())
			{
				ModelManager *modelManager = qPmApp->getModelManager();
				RosterModel *rosterModel = modelManager->rosterModel();
				if (rosterModel && (!rosterModel->isFriend(id)))
				{
					menu.addSeparator();
					menu.addAction(m_addFriend);
				}
			}
			else
			{
				menu.addAction(m_changeCardName);
			}

			if (m_menuDelegate)
			{
				m_menuDelegate->appendMenuItem(&menu, id);
			}

			// show menu
			menu.exec(QCursor::pos());
		}
	}
}

void GroupMemberView::chat()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (id == Account::instance()->id()) // check if is self
		return;

	emit chat(id);
}

void GroupMemberView::sendMail()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit sendMail(id);
}

void GroupMemberView::viewMaterial()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	emit viewMaterial(id);
}

void GroupMemberView::addFriend()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (!action)
		return;

	QString id = action->data().toString();
	if (id.isEmpty())
		return;

	if (id == Account::instance()->id()) // check if is self
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(id);
	emit addFriendRequest(id, name);
}

void GroupMemberView::atTA()
{
	QString uid = m_at->data().toString();
	if (!uid.isEmpty())
	{
		QString atName = m_groupMemberModel->memberNameInGroup(uid);
		QString atText = QString("%1(%2)").arg(atName).arg(uid);
		emit atTA(atText);
	}
}

void GroupMemberView::changeCardName()
{
	QString uid = m_changeCardName->data().toString();
	if (!uid.isEmpty())
	{
		QString origName = m_groupMemberModel->memberNameInGroup(uid);
		PlainTextLineInput cardNameInput(this);
		cardNameInput.init(tr("Change My Card Name"), tr("My card name:"), 20, PlainTextLineInput::ModeUnicode, origName, true);
		if (QDialog::Rejected == cardNameInput.exec())
			return;

		if (!qPmApp->GetLoginMgr()->isLogined())
		{
			PMessageBox::information(this, tr("Tip"), tr("You are offline, can't change card name"));
			return;
		}

		QString cardName = cardNameInput.getInputText().trimmed();
		emit changeCardName(uid, cardName);
	}
}

void GroupMemberView::setSkin()
{
	setStyleSheet("QListView{"
		"border: none;"
		"background-color: transparent;"
		"font-size: 9pt;"
		"}"
	);
}

void GroupMemberView::setupActions()
{
	m_chat = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chat, SIGNAL(triggered()), SLOT(chat()));

	m_sendMail = new QAction(/*QIcon(":/images/icon.png"),*/ tr("Send Mail"), this);
	connect(m_sendMail, SIGNAL(triggered()), SLOT(sendMail()));

	m_viewMaterial = new QAction(/*QIcon(":/images/icon.png"),*/ tr("View Contact Profile"), this);
	connect(m_viewMaterial, SIGNAL(triggered()), SLOT(viewMaterial()));

	m_addFriend = new QAction(tr("Add Friends"), this);
	connect(m_addFriend, SIGNAL(triggered()), SLOT(addFriend()));

	m_at = new QAction(tr("@ TA"), this);
	connect(m_at, SIGNAL(triggered()), this, SLOT(atTA()));

	m_changeCardName = new QAction(tr("Change My Card Name"), this);
	connect(m_changeCardName, SIGNAL(triggered()), this, SLOT(changeCardName()));
}
