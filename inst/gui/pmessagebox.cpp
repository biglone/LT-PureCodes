#include "pmessagebox.h"
#include "ui_pmessagebox.h"

QDialogButtonBox::StandardButton PMessageBox::critical(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons /*= QDialogButtonBox::Ok*/)
{
	PMessageBox msgBox(PMessageBox::Failed, text, buttons, title, parent);
	msgBox.exec();
	return msgBox.clickedButton();
}

QDialogButtonBox::StandardButton PMessageBox::information(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons /*= QDialogButtonBox::Ok*/)
{
	PMessageBox msgBox(PMessageBox::Information, text, buttons, title, parent);
	msgBox.exec();
	return msgBox.clickedButton();
}

QDialogButtonBox::StandardButton PMessageBox::question(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons /*= QDialogButtonBox::Ok*/)
{
	PMessageBox msgBox(PMessageBox::Question, text, buttons, title, parent);
	msgBox.exec();
	return msgBox.clickedButton();
}

QDialogButtonBox::StandardButton PMessageBox::warning(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons /*= QDialogButtonBox::Ok*/)
{
	PMessageBox msgBox(PMessageBox::Warning, text, buttons, title, parent);
	msgBox.exec();
	return msgBox.clickedButton();
}

QDialogButtonBox::StandardButton PMessageBox::success(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons /*= QDialogButtonBox::Ok*/)
{
	PMessageBox msgBox(PMessageBox::Success, text, buttons, title, parent);
	msgBox.exec();
	return msgBox.clickedButton();
}

PMessageBox::PMessageBox(Type type, const QString &tip, QDialogButtonBox::StandardButtons btns, const QString &title, QWidget *parent)
	: FramelessDialog(parent)
{
	ui = new Ui::PMessageBox();
	ui->setupUi(this);

	switch (type) {
		case Success:
			ui->labelIcon->setPixmap(QPixmap(":/messagebox/success.png"));
			break;
		case Failed:
			ui->labelIcon->setPixmap(QPixmap(":/messagebox/failed.png"));
			break;
		case Question:
			ui->labelIcon->setPixmap(QPixmap(":/messagebox/question.png"));
			break;
		case Information:
			ui->labelIcon->setPixmap(QPixmap(":/messagebox/info.png"));
			break;
		case Warning:
			ui->labelIcon->setPixmap(QPixmap(":/messagebox/warning.png"));
			break;
		default:
			ui->labelIcon->setPixmap(QPixmap(":/messagebox/success.png"));
			break;
	}

	m_clickedRole = QDialogButtonBox::RejectRole;
	m_clickedButton = QDialogButtonBox::Cancel;

	ui->labelTitle->setText(title);
	setWindowTitle(title);

	ui->labelTip->setText(tip);

	setMainLayout(ui->verticalLayoutMain);
	ui->buttonBox->setStandardButtons(btns);

	// compute the dialog height
	QStringList tipLines = tip.split(QChar('\n'));
	int lines = 0;
	QFontMetrics fontMetrics = ui->labelTip->fontMetrics();
	foreach (QString tipLine, tipLines)
	{
		int tipLen = fontMetrics.width(tipLine);
		lines += tipLen/240 + 1;
	}
	int lineHeight = (fontMetrics.height()+10) * (lines+1);
	if (lineHeight < 64)
		lineHeight = 64;
	int height = 90 + lineHeight;
	resize(340, height);

	setResizeable(false);

	setSkin();

	connect(ui->btnClose, SIGNAL(clicked()), SLOT(reject()));
	connect(ui->buttonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), SLOT(reject()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
}

PMessageBox::~PMessageBox()
{
	delete ui;
}

void PMessageBox::setSkin()
{
	// set background image
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	// set title label style
	ui->labelTitle->setStyleSheet("QLabel {font-size: 12pt; color: white;}");

	// bottom bar button style
	QFile qssFile;
	QString btnQss;
	qssFile.setFileName(":/theme/qss/pushbutton2_skin.qss");
	if (qssFile.open(QIODevice::ReadOnly))
	{
		btnQss = qssFile.readAll();
		qssFile.close();
	}

	foreach (QAbstractButton *btn, ui->buttonBox->buttons())
	{
		btn->setStyleSheet(btnQss);
	}
}

void PMessageBox::buttonClicked(QAbstractButton *button)
{
	m_clickedRole   = ui->buttonBox->buttonRole(button);
	m_clickedButton = ui->buttonBox->standardButton(button);
}