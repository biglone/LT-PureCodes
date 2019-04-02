#include "selectchatdialog.h"
#include "ui_selectchatdialog.h"
#include "ModelManager.h"
#include "PmApp.h"
#include "pmessagebox.h"
#include "editfilterwidget.h"
#include "editfiltertreeview.h"
#include "Account.h"

SelectChatDialog::SelectChatDialog(const QString &title, QWidget *parent /*= 0*/)
	: FramelessDialog(parent)
{
	ui = new Ui::SelectChatDialog();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	ui->title->setText(title);
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(460, 440);
	setResizeable(false);

	initUI();

	connect(ui->leditFilter, SIGNAL(filterChanged(QString)), ui->selectChatWidget, SLOT(editFilterChanged(QString)));
	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->selectChatWidget, SIGNAL(itemClicked(QString, int)), this, SLOT(onChatSelected(QString, int)));

	setSkin();
}

SelectChatDialog::~SelectChatDialog()
{
	delete ui;
}

void SelectChatDialog::getSelect(bean::MessageType &selType, QString &selId)
{
	selType = m_selectType;
	selId = m_selectId;
}

void SelectChatDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_3.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 31;
	setBG(bgPixmap, bgSizes);

	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");
	ui->searchBar->setStyleSheet("QWidget#searchBar {background-color: rgb(232, 232, 232); border-bottom: 1px solid rgb(219, 219, 219);}");
}

void SelectChatDialog::onChatSelected(const QString &id, int source)
{
	if (id == Account::instance()->id())
	{
		PMessageBox::warning(this, tr("Tip"), tr("Can't forward to self"));
		return;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	QString name;
	m_selectId = id;
	if (source == SelectChatWidget::RosterSource)
	{
		m_selectType = bean::Message_Chat;
		name = modelManager->userName(id);
	}
	else if (source == SelectChatWidget::OsSource)
	{
		m_selectType = bean::Message_Chat;
		name = modelManager->userName(id);
	}
	else if (source == SelectChatWidget::GroupSource)
	{
		m_selectType = bean::Message_GroupChat;
		name = modelManager->groupName(id);
	}
	else if (source == SelectChatWidget::DiscussSource)
	{
		m_selectType = bean::Message_DiscussChat;
		name = modelManager->discussName(id);
	}

	if (!name.isEmpty())
	{
		QDialogButtonBox::StandardButton ret = PMessageBox::question(this, tr("Forward"), 
			tr("Forward to: %1").arg(name), QDialogButtonBox::Yes|QDialogButtonBox::No);
		if (ret == QDialogButtonBox::Yes)
		{
			accept();
		}
	}
}

void SelectChatDialog::initUI()
{
	// select widget
	ui->selectChatWidget->init(QString(), true);
	EditFilterWidget *searchWidget = ui->selectChatWidget->searchWidget();
	searchWidget->setTipTextVisible(false);
	searchWidget->setSelectMode(EditFilterWidget::SingleClick);
	ui->selectChatWidget->selectFirst();

	// filter line edit
	QIcon icon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
		QLatin1String("edit-clear-locationbar-rtl") :
	QLatin1String("edit-clear-locationbar-ltr"),
		QIcon::fromTheme(QLatin1String("edit-clear"), QIcon(QLatin1String(":/images/Icon_105.png"))));
	ui->leditFilter->setButtonPixmap(FilterLineEdit::Left, icon.pixmap(16));
	ui->leditFilter->setButtonVisible(FilterLineEdit::Left, true);
	ui->leditFilter->setAutoHideButton(FilterLineEdit::Left, false);
	ui->leditFilter->setPlaceholderText(tr("Search"));
	ui->leditFilter->setKeyDelegate(ui->selectChatWidget->searchWidget()->treeViewSearch());
}

