#include "dateperiodpicker.h"
#include "ui_dateperiodpicker.h"
#include "calendarwidget.h"

DatePeriodPicker::DatePeriodPicker(const QDate &from, const QDate &to, QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::DatePeriodPicker();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());
	setWindowTitle(ui->title->text());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(332, 188);
	setResizeable(false);

	ui->datePeriodWidget->setPeriod(from, to);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

	setSkin();
}

DatePeriodPicker::~DatePeriodPicker()
{
	delete ui;
}

QDate DatePeriodPicker::from() const
{
	return ui->datePeriodWidget->from();
}

QDate DatePeriodPicker::to() const
{
	return ui->datePeriodWidget->to();
}

void DatePeriodPicker::setSkin()
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
		ui->btnOK->setStyleSheet(qss);
		ui->btnCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void DatePeriodPicker::accept()
{
	if (!ui->datePeriodWidget->validatePeriod())
		return;

	QDialog::accept();
}
