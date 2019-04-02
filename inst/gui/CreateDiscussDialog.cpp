#include <QTimer>
#include <QDebug>
#include "PmApp.h"
#include "buddymgr.h"
#include "login/Account.h"

#include "model/ModelManager.h"
#include "model/rostermodeldef.h"
#include "model/orgstructmodeldef.h"
#include "pmessagebox.h"
#include "manager/DiscussManager.h"
#include "model/DiscussModeldef.h"
#include "model/groupitemlistmodeldef.h"
#include "commoncombobox.h"
#include "settings/GlobalSettings.h"

#include <QCompleter>
#include <QStandardItemModel>
#include <QTreeView>

#include "CreateDiscussDialog.h"
#include "showredundantdiscussmemberdialog.h"
#include "ui_CreateDiscussDialog.h"

static const int kIdRole = Qt::UserRole + 1;
static const int kNameRole = Qt::UserRole + 2;

static const int KMaxNameLen = 50;

CreateDiscussDialog::CreateDiscussDialog(ActionType type /*= Type_Create*/, const QString &discussName /*= ""*/, 
										 const QStringList &baseUids /*= QStringList()*/, const QStringList &preAddUids /*= QStringList()*/, 
										 bool modifyState /*= false*/, QWidget *parent /*= 0*/)
: FramelessDialog(parent)
, m_type(type)
, m_lstBaseUids(baseUids)
{
	ui = new Ui::CreateDiscussDialog();
	ui->setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	QStringList preAddIds = preAddUids;
	if (m_type == Type_Create || m_type == Type_CreateInterphone)
	{
		ui->title->setText(tr("Create Discuss"));
		ui->leditName->setText(discussName);
		ui->labelNameRule->setText(tr("Name should be within %1 characters").arg(KMaxNameLen));
		ui->nameBar->setVisible(true);
	}
	else if (m_type == Type_Add)
	{
		ui->title->setText(tr("Invite Discuss Member") + " - " + discussName);
		ui->leditName->setText(discussName);
		ui->leditName->setReadOnly(true);
		ui->nameBar->setVisible(false);
	}

	QString myId = Account::instance()->id();
	if (!m_lstBaseUids.contains(myId))
	{
		m_lstBaseUids.prepend(myId);
	}
	else
	{
		m_lstBaseUids.removeAll(myId);
		m_lstBaseUids.prepend(myId);
	}

	if (!preAddIds.contains(myId))
	{
		preAddIds.prepend(myId);
	}
	else
	{
		preAddIds.removeAll(myId);
		preAddIds.prepend(myId);
	}

	// init select widget
	ui->widgetSelect->init(SelectContactWidget::SelectAll, QStringList(), QStringList(), preAddIds, m_lstBaseUids, GlobalSettings::maxDiscussMemberCount());

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	resize(658, 568);
	setMinimumSize(658, 568);
	setResizeable(true);
	setMaximizeable(true);

	initUI(modifyState);

	initSignals();

	setSkin();
}

CreateDiscussDialog::~CreateDiscussDialog()
{
	delete ui;
}

void CreateDiscussDialog::setDiscussId(const QString &id)
{
	m_discussId = id;
}

QString CreateDiscussDialog::discussId() const
{
	return m_discussId;
}

void CreateDiscussDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {color: white; font-size: 12pt;}");

	// aristic&rule label style
	ui->labelAristic->setStyleSheet("QLabel {color: red;}");
	ui->labelNameRule->setStyleSheet("QLabel {font: 9pt;}");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->pushButtonOK->setStyleSheet(qss);
		ui->pushButtonClose->setStyleSheet(qss);
		qssFile.close();
	}
}

void CreateDiscussDialog::closeDialog()
{
	close();
}

void CreateDiscussDialog::addDiscussMembers()
{
	ModelManager *modelManager = qPmApp->getModelManager();
	QStringList discussIds = modelManager->discussModel()->allDiscussIds();

	QStringList ids = ui->widgetSelect->selectionIds();
	QSet<QString> newMemberIdsSet = ids.toSet();
	int maxDiscussMemberCount = GlobalSettings::maxDiscussMemberCount();
	if (ids.length() > maxDiscussMemberCount)
	{
		PMessageBox::information(this, tr("Information"), tr("The members of %1 have exceeded %2.")
			.arg(tr("Discuss")).arg(maxDiscussMemberCount));
		return;
	}

	if (m_type == Type_Create)
	{
		QString name = ui->leditName->text().trimmed();
		if (!ids.contains(Account::instance()->id()))
		{
			ids.append(Account::instance()->id());
		}

		QStringList exittingDiscussIds = QStringList();
		foreach(QString discussId, discussIds)
		{
			GroupItemListModel *pModel = modelManager->discussItemsModel(discussId);
			if (!pModel)
				continue;

			QStringList specificDiscussMemberIds = pModel->allMemberIds();
			QSet<QString> idsSet = specificDiscussMemberIds.toSet();
			if (idsSet == newMemberIdsSet)
			{
				exittingDiscussIds << discussId;
			}
		}
		if (!exittingDiscussIds.isEmpty())
		{
			ShowRedundantDiscussMemberDialog *srdmDialog = new ShowRedundantDiscussMemberDialog(exittingDiscussIds);
			if (srdmDialog->exec() == QDialog::Accepted)
			{
				emit createDiscuss(name, ids);
			}
		}
		else
		{
			emit createDiscuss(name, ids);
		}
	}
	else if (m_type == Type_CreateInterphone)
	{
		if (!ids.contains(Account::instance()->id()))
		{
			ids.append(Account::instance()->id());
		}

		QString name = ui->leditName->text().trimmed();

		emit createInterphone(name, ids);
	}
	else
	{
		foreach (QString id, m_lstBaseUids)
		{
			ids.removeAll(id);
		}

		emit addMembers(m_discussId, ids);
	}

	closeDialog();
}

void CreateDiscussDialog::onSelectionChanged()
{
	QStringList selIds = ui->widgetSelect->selectionIds();

	// button status
	foreach (QString uid, m_lstBaseUids)
	{
		selIds.removeAll(uid);
	}

	QString name = ui->leditName->text().trimmed();
	if (!name.isEmpty() && !selIds.isEmpty())
	{
		ui->pushButtonOK->setEnabled(true);
	}
	else
	{
		ui->pushButtonOK->setEnabled(false);
	}
}

void CreateDiscussDialog::onMaximizeStateChanged(bool isMaximized)
{
	if (isMaximized)
	{
		ui->btnMaximize->setChecked(true);
		ui->btnMaximize->setToolTip(tr("Restore"));
	}
	else
	{
		ui->btnMaximize->setChecked(false);
		ui->btnMaximize->setToolTip(tr("Maximize"));
	}
}

void CreateDiscussDialog::nameTextChanged(const QString &text)
{
	int cursorPos = ui->leditName->cursorPosition();

	QString endText = text;
	while (endText.length() > KMaxNameLen)
	{
		endText = endText.left(endText.length()-1);
	}
	ui->leditName->setText(endText);

	// recover the cursor pos
	if (cursorPos < endText.length())
	{
		ui->leditName->setCursorPosition(cursorPos);
	}

	// check selection
	onSelectionChanged();
}

void CreateDiscussDialog::completerActivated(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	QString discussId = index.data(kIdRole).toString();
	if (!discussId.isEmpty())
	{
		emit openDiscuss(discussId);
	}
}

void CreateDiscussDialog::onDiscussListChanged()
{
	addNameCompleterModel();
}

void CreateDiscussDialog::initUI(bool modifyState)
{
	initNameCompleter();
	ui->leditName->setPlaceholderText(tr("Please input name of discuss"));

	if (!modifyState)
	{
		ui->pushButtonOK->setEnabled(false);
	}
	else
	{
		ui->pushButtonOK->setEnabled(true);
	}
}

void CreateDiscussDialog::initSignals()
{
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(closeDialog()));
	connect(ui->btnMinimize, SIGNAL(clicked()), this, SLOT(showMinimized()));
	connect(ui->btnMaximize, SIGNAL(clicked()), this, SLOT(triggerMaximize()));
	connect(ui->pushButtonClose, SIGNAL(clicked()), this, SLOT(closeDialog()));
	connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(addDiscussMembers()));
	connect(ui->widgetSelect, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
	connect(ui->leditName, SIGNAL(textChanged(QString)), this, SLOT(nameTextChanged(QString)));
}

void CreateDiscussDialog::initNameCompleter()
{
	m_completerModel = new QStandardItemModel(this);
	addNameCompleterModel();

	// header style
	m_completerView = new QTreeView(this);
	m_completerView->header()->setStyleSheet(QString("QHeaderView::section {"
		"font-size: 10.5pt;"
		"color: rgb(128, 128, 128);"
		"background-color: white;"
		"padding-left: 12px;"
		"border: none;"
		"}"
		));
	m_completerView->header()->setSectionResizeMode(QHeaderView::Stretch);
	m_completerView->header()->setSortIndicatorShown(false);
	m_completerView->header()->setSectionsClickable(false);
	m_completerView->header()->setMinimumHeight(35);
	m_completerView->setHeaderHidden(false);

	// completer
	m_completer = new QCompleter(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setPopup(m_completerView);
	m_completer->setModel(m_completerModel);
	m_completer->setModelSorting(QCompleter::UnsortedModel);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setWrapAround(true);
	m_completer->setCompletionRole(kNameRole);
	m_completer->setMaxVisibleItems(8);

	// view
	m_completerView->setMinimumHeight(120);
	m_completerView->setIndentation(0);
	m_completerView->setAttribute(Qt::WA_Hover, true);
	m_completerView->setMouseTracking(true);
	m_completerView->setItemDelegate(new ComboItemDelegate(m_completer->popup()));

	ui->leditName->setCompleter(m_completer);
	connect(m_completer, SIGNAL(activated(QModelIndex)), this, SLOT(completerActivated(QModelIndex)));

	ModelManager *modelManager = qPmApp->getModelManager();
	DiscussManager *discussManager = qPmApp->getDiscussManager();
	connect(discussManager, SIGNAL(notifyDiscussChanged(QString)), this, SLOT(onDiscussListChanged()));
	DiscussModel *discussModel = modelManager->discussModel();
	connect(discussModel, SIGNAL(discussDeleted(QString)), this, SLOT(onDiscussListChanged()));
}

void CreateDiscussDialog::addNameCompleterModel()
{
	m_completerModel->clear();
	ModelManager *modelManager = qPmApp->getModelManager();
	DiscussModel *discussModel = modelManager->discussModel();
	QStringList discussIds = discussModel->allDiscussIds();
	foreach (QString discussId, discussIds)
	{
		QStandardItem *item = new QStandardItem();
		QString discussName = modelManager->discussName(discussId);
		QString discussText = discussName;
		GroupItemListModel *memberModel = modelManager->discussItemsModel(discussId);
		if (memberModel)
		{
			int memberCount = memberModel->memberCount();
			if (memberCount > 0)
				discussText = tr("%1(%2 members)").arg(discussName).arg(memberCount);
		}
		item->setText(discussText);
		item->setData(discussId, kIdRole);
		item->setData(discussName, kNameRole);
		m_completerModel->invisibleRootItem()->appendRow(item);
	}

	QStandardItem *headerItem = new QStandardItem();
	headerItem->setText(tr("Already created discusses:"));
	headerItem->setSizeHint(QSize(100, 35));
	m_completerModel->setHorizontalHeaderItem(0, headerItem);
}
