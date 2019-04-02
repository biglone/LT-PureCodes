#include "recordbar.h"
#include "ui_recordbar.h"
#include <QPainter>

RecordBar::RecordBar(QWidget *parent)
	: QWidget(parent), m_recordTimeInMs(0), m_running(false)
{
	ui = new Ui::RecordBar();
	ui->setupUi(this);
}

RecordBar::~RecordBar()
{
	delete ui;
}

void RecordBar::setRecordTime(int ms)
{
	m_recordTimeInMs = ms;
	if (m_recordTimeInMs >= 60*1000)
		m_recordTimeInMs = 60*1000;
	if (m_recordTimeInMs < 0)
		m_recordTimeInMs = 0;
	update();
}

int RecordBar::recordTime() const
{
	return m_recordTimeInMs;
}

void RecordBar::setRunning(bool running)
{
	m_running = running;
}

bool RecordBar::isRunning() const
{
	return m_running;
}

void RecordBar::paintEvent(QPaintEvent * /*e*/)
{
	QPainter painter(this);

	// draw background
	painter.setBrush(QBrush(QColor(223, 230, 236)));
	painter.setPen(Qt::NoPen);
	painter.drawRect(rect());

	// draw bottom line
	painter.setPen(QColor(219, 219, 219));
	painter.setBrush(Qt::NoBrush);
	painter.drawLine(rect().bottomLeft(), rect().bottomRight());

	// draw time rect
	QPoint topLeft = rect().topLeft();
	int width = (int)((m_recordTimeInMs/60000.0)*rect().width());
	QPoint bottomRight = rect().bottomRight();
	bottomRight.setX(topLeft.x() + width);
	bottomRight.setY(bottomRight.y() - 1);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(190, 232, 248)));
	painter.drawRect(QRect(topLeft, bottomRight));
}

void RecordBar::on_pushButtonSend_clicked()
{
	emit sendRecord();
}

void RecordBar::on_pushButtonCancel_clicked()
{
	emit cancelRecord();
}
