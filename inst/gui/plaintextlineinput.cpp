#include "plaintextlineinput.h"
#include "ui_plaintextlineinput.h"
#include <QByteArray>

static const int DEFAULT_MAX_LENGTH = 1024;

PlainTextLineInput::PlainTextLineInput(QWidget *parent)
: FramelessDialog(parent), m_maxLength(DEFAULT_MAX_LENGTH), m_allowEmpty(false)
{
	ui = new Ui::PlainTextLineInput();
	ui->setupUi(this);

	setWindowIcon(qApp->windowIcon());

	setMainLayout(ui->verticalLayoutMain);
	setFixedSize(284, 158);
	setResizeable(false);

	ui->input->setMaxLength(DEFAULT_MAX_LENGTH);

	connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	connect(ui->input, SIGNAL(textChanged(QString)), this, SLOT(inputTextChanged(QString)));

	ui->pushButtonOK->setEnabled(false);

	setSkin();
}

PlainTextLineInput::~PlainTextLineInput()
{
	delete ui;
}

void PlainTextLineInput::init(const QString &title, const QString &tip, int maxLength, MaxLengthMode mode, 
							  const QString &placeText /*= QString()*/, bool allowEmpty /*= false*/)
{
	ui->title->setText(title);
	setWindowTitle(ui->title->text());

	ui->labelTip->setText(tip);

	m_maxLength = maxLength;
	m_maxLengthMode = mode;
	if (m_maxLengthMode == ModeUnicode)
		ui->input->setMaxLength(m_maxLength);
	m_allowEmpty = allowEmpty;

	ui->input->setText(placeText);

	if (ui->input->text().isEmpty() && !m_allowEmpty)
	{
		ui->pushButtonOK->setEnabled(false);
	}
	else
	{
		ui->pushButtonOK->setEnabled(true);
	}
}

QString PlainTextLineInput::getInputText() const
{
	return ui->input->text();
}

void PlainTextLineInput::setSkin()
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

void PlainTextLineInput::inputTextChanged(const QString &text)
{
	if (m_maxLengthMode == ModeUtf8)
	{
		int cursorPos = ui->input->cursorPosition();
		
		QString endText = text;
		QByteArray utf8Text = endText.toUtf8();
		while (utf8Text.length() > m_maxLength)
		{
			endText = endText.left(endText.length()-1);
			utf8Text = endText.toUtf8();
		}
		ui->input->setText(endText);

		// recover the cursor pos
		if (cursorPos < endText.length())
			ui->input->setCursorPosition(cursorPos);
	}

	if (text.isEmpty() && !m_allowEmpty)
	{
		ui->pushButtonOK->setEnabled(false);
	}
	else
	{
		ui->pushButtonOK->setEnabled(true);
	}
}