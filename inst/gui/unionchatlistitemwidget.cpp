#include "unionchatlistitemwidget.h"
#include "ui_unionchatlistitemwidget.h"
#include <QPainter>
#include <QDebug>
#include <QStandardItem>
#include <QListView>
#include "unionchatdialog.h"

UnionChatListItemWidget::UnionChatListItemWidget(QStandardItem *item, QListView *listView, QWidget *parent)
	: QWidget(parent), m_item(item), m_listView(listView), m_isHover(false)
{
	ui = new Ui::UnionChatListItemWidget();
	ui->setupUi(this);

	ui->labelIcon->setFixedSize(QSize(40, 40));
	ui->labelName->setStyleSheet("QLabel#labelName{color: white;}");
	ui->toolButtonClose->setVisible(false);

	StyleToolButton::Info info;
	info.urlNormal = QString(":/images/Icon_129.png");
	info.urlHover = QString(":/images/Icon_129_pressed.png");
	info.urlPressed = QString(":/images/Icon_129_pressed.png");
	ui->toolButtonClose->setInfo(info);

	connect(ui->toolButtonClose, SIGNAL(clicked()), this, SIGNAL(closeChat()));

	Q_ASSERT(m_listView);
	Q_ASSERT(m_item);

	QPixmap pixmap = m_item->icon().pixmap(ui->labelIcon->size());
	ui->labelIcon->setPixmap(pixmap);

	QString name = m_item->text();
	ui->labelName->setText(name);
}

UnionChatListItemWidget::~UnionChatListItemWidget()
{
	delete ui;
}

void UnionChatListItemWidget::refresh()
{
	if (!m_item)
		return;

	QPixmap pixmap = m_item->icon().pixmap(ui->labelIcon->size());
	ui->labelIcon->setPixmap(pixmap);

	QString name = m_item->text();
	ui->labelName->setText(name);

	update();
}

void UnionChatListItemWidget::setItem(QStandardItem *item)
{
	m_item = item;
}

void UnionChatListItemWidget::paintEvent(QPaintEvent *ev)
{
	if (!m_item)
	{
		QWidget::paintEvent(ev);
		return;
	}

	QPainter painter(this);
	if (m_listView->currentIndex() == m_item->index()) // selected
	{
		QRect rt = rect();
		painter.fillRect(rt, QColor("#108bed"));
	}
	else
	{
		int unreadMessageCount = 0;
		QVariant v = m_item->data(kChatListMsgCountRole);
		if (v.isValid())
			unreadMessageCount = v.toInt();
		if (unreadMessageCount > 0 && !m_isHover)
		{
			// draw unread message count
			QRect rt = rect();
			QRect msgCountRect;
			msgCountRect = rt;
			msgCountRect.setRight(rt.right() - 5);
			msgCountRect.setLeft(rt.right() - 25);
			msgCountRect.setTop(rt.top() + (rt.height() - 14)/2);
			msgCountRect.setHeight(14);

			// draw background
			QPixmap unreadBack(":/images/unread_back.png");
			painter.drawPixmap(msgCountRect.topLeft(), unreadBack);

			// draw count
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QColor(255, 255, 255));
			QFont originalFont = painter.font();
			QFont smallFont = originalFont;
			smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/4);
			painter.setFont(smallFont);
			QString unreadMsgCountText = QString::number(unreadMessageCount);
			if (unreadMessageCount > 99)
				unreadMsgCountText = QString("99+");
			painter.drawText(msgCountRect, Qt::AlignHCenter|Qt::AlignVCenter, unreadMsgCountText);
			painter.setFont(originalFont);
		}
	}
}

void UnionChatListItemWidget::enterEvent(QEvent *ev)
{
	ui->toolButtonClose->setVisible(true);
	m_isHover = true;
	update();

	QWidget::enterEvent(ev);
}

void UnionChatListItemWidget::leaveEvent(QEvent *ev)
{
	QWidget::leaveEvent(ev);

	ui->toolButtonClose->setVisible(false);
	m_isHover = false;
	update();
}

void UnionChatListItemWidget::resizeEvent(QResizeEvent *ev)
{
	QWidget::resizeEvent(ev);

	if (ev->size().width() < 100)
		ui->labelName->setVisible(false);
	else
		ui->labelName->setVisible(true);
}