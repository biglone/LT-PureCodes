#include "autotipwidget.h"
#include "ui_autotipwidget.h"
#include <QPainter>

const int AUTO_SHOW_SECONDS = 5; // 5s

AutoTipWidget::AutoTipWidget(QWidget *parent)
	: QWidget(parent), m_backgroundColor(223, 238, 250)
{
	ui = new Ui::AutoTipWidget();
	ui->setupUi(this);

	ui->labelText->setStyleSheet("color: #333333;");

	StyleToolButton::Info info;
	info.urlNormal = ":/images/close_normal.png";
	info.urlHover = ":/images/close_pressed.png";
	info.urlPressed = ":/images/close_pressed.png";
	info.tooltip = tr("Close");
	ui->toolButtonClose->setInfo(info);

	m_autoHideTimer.setSingleShot(true);
	m_showSeconds = AUTO_SHOW_SECONDS;
	connect(&m_autoHideTimer, SIGNAL(timeout()), this, SLOT(onShowTimer()));
	connect(ui->toolButtonClose, SIGNAL(clicked()), this, SLOT(stopShow()));
}

AutoTipWidget::~AutoTipWidget()
{
	delete ui;
}

void AutoTipWidget::setTipPixmap(const QPixmap &pixmap)
{
	ui->labelIcon->setPixmap(pixmap);
}

void AutoTipWidget::setTipText(const QString &text)
{
	ui->labelText->setText(text);
}

void AutoTipWidget::setTipTextColor(const QColor &color)
{
	ui->labelText->setStyleSheet(QString("color: %1;").arg(color.name()));
}

void AutoTipWidget::setBackgroundColor(const QColor &color)
{
	m_backgroundColor = color;
}

void AutoTipWidget::setCanClose(bool canClose)
{
	if (canClose)
		ui->toolButtonClose->setVisible(true);
	else
		ui->toolButtonClose->setVisible(false);
}

void AutoTipWidget::autoShow()
{
	if (m_autoHideTimer.isActive())
		m_autoHideTimer.stop();
	m_autoHideTimer.start(m_showSeconds*1000);
	show();
}

void AutoTipWidget::stopShow()
{
	m_autoHideTimer.stop();
	hide();
}

void AutoTipWidget::onShowTimer()
{
	hide();
}

void AutoTipWidget::paintEvent(QPaintEvent * /*ev*/)
{
	QPainter painter(this);
	painter.setBrush(QBrush(m_backgroundColor));
	painter.setPen(Qt::NoPen);
	painter.drawRect(rect());
}