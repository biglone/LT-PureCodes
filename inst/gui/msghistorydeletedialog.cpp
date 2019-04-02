#include "msghistorydeletedialog.h"
#include "ui_msghistorydeletedialog.h"
#include "common/datetime.h"
#include "dateperiodpicker.h"
#include "pmessagebox.h"

static const int FIXED_WIDTH       = 364;
static const int FIXED_HEIGHT_PART = 179;
static const int FIXED_HEIGHT_FULL = 219;

MsgHistoryDeleteDialog::MsgHistoryDeleteDialog(QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::MsgHistoryDeleteDialog();
	ui->setupUi(this);

	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(FIXED_WIDTH, FIXED_HEIGHT_PART);
	setResizeable(false);

	QDate dtEnd = CDateTime::currentDateTime().date();
	QDate dtFrom = dtEnd.addDays(-7);
	ui->datePeriodWidget->setPeriod(dtFrom, dtEnd);
	ui->datePeriodLine->setVisible(false);

	initComboBoxDate();

	ui->radioButtonDeleteAll->setChecked(true);
	ui->radioButtonDeleteDate->setChecked(false);
	ui->comboBoxDate->setEnabled(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnDelete, SIGNAL(clicked()), this, SLOT(onBtnDeleteClicked()));

	setSkin();
}

MsgHistoryDeleteDialog::~MsgHistoryDeleteDialog()
{
	delete ui;
}

MsgHistoryDeleteDialog::DeleteAction MsgHistoryDeleteDialog::deleteAction() const
{
	DeleteAction delAction = DeleteAll;
	if (ui->radioButtonDeleteDate->isChecked())
	{
		delAction = (DeleteAction)(ui->comboBoxDate->currentIndex());
	}
	return delAction;
}

void MsgHistoryDeleteDialog::getDatePeriod(QDate &beginDate, QDate &endDate)
{
	beginDate = ui->datePeriodWidget->from();
	endDate = ui->datePeriodWidget->to();
}

void MsgHistoryDeleteDialog::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->title->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// bottom bar button style
	QFile qssFile;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		QString qss = qssFile.readAll();
		ui->btnDelete->setStyleSheet(qss);
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void MsgHistoryDeleteDialog::on_radioButtonDeleteDate_toggled(bool checked)
{
	if (checked)
	{
		ui->radioButtonDeleteAll->setChecked(false);
		ui->comboBoxDate->setEnabled(true);
	}
	else
	{
		ui->radioButtonDeleteAll->setChecked(true);
		ui->comboBoxDate->setEnabled(false);
	}
}

void MsgHistoryDeleteDialog::onComboBoxDateActivated(int index)
{
	if (index == (int)DeleteDatePeriod)
	{
		setFixedSize(FIXED_WIDTH, FIXED_HEIGHT_FULL);
		resize(FIXED_WIDTH, FIXED_HEIGHT_FULL);
		ui->datePeriodLine->setVisible(true);
	}
	else
	{
		ui->datePeriodLine->setVisible(false);
		setFixedSize(FIXED_WIDTH, FIXED_HEIGHT_PART);
		resize(FIXED_WIDTH, FIXED_HEIGHT_PART);
	}
}

void MsgHistoryDeleteDialog::onBtnDeleteClicked()
{
	if (ui->comboBoxDate->currentIndex() == (int)DeleteDatePeriod)
	{
		if (!ui->datePeriodWidget->validatePeriod())
			return;
	}

	PMessageBox msgBox(PMessageBox::Question, tr("Are you sure to delete related chat history"), QDialogButtonBox::Yes|QDialogButtonBox::No, tr("Delete Chat History"), this);
	msgBox.setWindowModality(Qt::WindowModal);
	msgBox.exec();
	if (msgBox.clickedButton() == QDialogButtonBox::Yes)
	{
		accept();
	}
}

void MsgHistoryDeleteDialog::initComboBoxDate()
{
	QStringList slistItem;
	slistItem 
		<< tr("One Day Ago")
		<< tr("One Week Ago")
		<< tr("One Month Ago")
		<< tr("Three Months Ago")
		<< tr("Half Year Ago")
		<< tr("One Year Ago")
		<< tr("A Period");

	ui->comboBoxDate->addItems(slistItem);
	ui->comboBoxDate->setCurrentIndex(0);
	connect(ui->comboBoxDate, SIGNAL(activated(int)), this, SLOT(onComboBoxDateActivated(int)));
}

