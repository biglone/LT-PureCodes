#include "plaintextinput.h"
#include "ui_plaintextinput.h"

static const int DEFAULT_MAX_LENGTH = 1024;

PlainTextInput::PlainTextInput(QWidget *parent)
	: FramelessDialog(parent), m_maxLength(DEFAULT_MAX_LENGTH)
{
	ui = new Ui::PlainTextInput();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(352, 232);
	setResizeable(false);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	connect(ui->input, SIGNAL(textChanged()), this, SLOT(inputTextChanged()));

	ui->pushButtonOK->setEnabled(false);

	setSkin();
}

PlainTextInput::~PlainTextInput()
{
	delete ui;
}

void PlainTextInput::init(const QString &title, const QString &tip, int maxLength, const QString &placeText /*= QString()*/)
{
	ui->title->setText(title);
	setWindowTitle(ui->title->text());

	ui->labelTip->setText(tip);

	m_maxLength = maxLength;

	ui->input->setPlainText(placeText);
	ui->input->moveCursor(QTextCursor::End);

	if (ui->input->toPlainText().isEmpty())
	{
		ui->pushButtonOK->setEnabled(false);
	}
	else
	{
		ui->pushButtonOK->setEnabled(true);
	}
}

QString PlainTextInput::getInputText() const
{
	return ui->input->toPlainText();
}

void PlainTextInput::setSkin()
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
		ui->pushButtonOK->setStyleSheet(qss);
		ui->pushButtonCancel->setStyleSheet(qss);
		qssFile.close();
	}
}

void PlainTextInput::inputTextChanged()
{
	QString text = ui->input->toPlainText();
	if (text.length() > m_maxLength)
	{
		text = text.left(m_maxLength);
		ui->input->setPlainText(text);
		ui->input->moveCursor(QTextCursor::End);
	}

	if (text.isEmpty())
	{
		ui->pushButtonOK->setEnabled(false);
	}
	else
	{
		ui->pushButtonOK->setEnabled(true);
	}
}
