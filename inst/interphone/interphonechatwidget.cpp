#include "interphonechatwidget.h"
#include "ui_interphonechatwidget.h"
#include <QPainter>

InterphoneChatWidget::InterphoneChatWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::InterphoneChatWidget();
	ui->setupUi(this);

	setInInterphone(false, QString());

	StylePushButton::Info info;
	info.urlNormal = QString(":/interphone/add_normal.png");
	info.urlHover = QString(":/interphone/add_pressed.png");
	info.urlPressed = QString(":/interphone/add_pressed.png");
	ui->pushButtonAdd->setInfo(info);
	ui->pushButtonAdd->setTextWithColor(tr("Add"), QColor(255, 255, 255).name());

	info.urlNormal = QString(":/interphone/exit_normal.png");
	info.urlHover = QString(":/interphone/exit_pressed.png");
	info.urlPressed = QString(":/interphone/exit_pressed.png");
	ui->pushButtonQuit->setInfo(info);
	ui->pushButtonQuit->setTextWithColor(tr("Quit"), QColor(255, 255, 255).name());

	info.urlNormal = QString(":/interphone/open_normal.png");
	info.urlHover = QString(":/interphone/open_pressed.png");
	info.urlPressed = QString(":/interphone/open_pressed.png");
	ui->pushButtonOpen->setInfo(info);
	ui->pushButtonOpen->setTextWithColor(tr("Open"), QColor(255, 255, 255).name());

	connect(ui->pushButtonAdd, SIGNAL(clicked()), this, SIGNAL(addInterphone()));
	connect(ui->pushButtonOpen, SIGNAL(clicked()), this, SIGNAL(openInterphone()));
	connect(ui->pushButtonQuit, SIGNAL(clicked()), this, SIGNAL(quitInterphone()));
}

InterphoneChatWidget::~InterphoneChatWidget()
{
	delete ui;
}

bool InterphoneChatWidget::isInInterphone() const
{
	return m_isInInterphone;
}

void InterphoneChatWidget::setInInterphone(bool in, const QString &tip)
{
	m_isInInterphone = in;

	ui->labelTip->setText(tip);

	if (!m_isInInterphone)
	{
		ui->labelIcon->setPixmap(QPixmap(":/interphone/chatsound.png"));

		ui->pushButtonAdd->setVisible(true);
		ui->pushButtonOpen->setVisible(false);
		ui->pushButtonQuit->setVisible(false);
	}
	else
	{
		ui->labelIcon->setPixmap(QPixmap(":/interphone/chatsound2.png"));

		ui->pushButtonAdd->setVisible(false);
		ui->pushButtonOpen->setVisible(true);
		// ui->pushButtonQuit->setVisible(true);
		ui->pushButtonQuit->setVisible(false); // do not quit here, has problem because of message box upon the interphone dialog
	}

	update();
}

void InterphoneChatWidget::paintEvent(QPaintEvent * /*ev*/)
{
	QPainter painter(this);
	
	if (!m_isInInterphone)
	{
		painter.setBrush(QColor(244, 239, 220));
		painter.setPen(QColor(200, 195, 163));
	}
	else
	{
		painter.setBrush(QColor(227, 235, 220));
		painter.setPen(QColor(168, 178, 153));
	}

	QRect rt = rect();
	rt = rt.adjusted(1, 0, -1, -1);
	painter.drawRect(rt);
}