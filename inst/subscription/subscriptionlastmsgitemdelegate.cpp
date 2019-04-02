#include "subscriptionlastmsgitemdelegate.h"
#include "PmApp.h"
#include "ModelManager.h"
#include <QPainter>
#include <QListView>
#include "subscriptionmsgmanager.h"
#include "subscriptionlastmsgmodel.h"
#include <QStandardItem>
#include <QFont>
#include "common/datetime.h"

SubscriptionLastMsgItemDelegate::SubscriptionLastMsgItemDelegate(SubscriptionLastMsgModel *model, QObject *parent)
	: QItemDelegate(parent), m_model(model)
{
	Q_ASSERT(m_model);
}

SubscriptionLastMsgItemDelegate::~SubscriptionLastMsgItemDelegate()
{

}

QSize SubscriptionLastMsgItemDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
	return QSize(68, 68);
}

void SubscriptionLastMsgItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QListView *listView = (QListView *)(this->parent());
	if (!listView)
		return;

	if (!index.isValid())
		return;

	QStandardItem *item = m_model->itemFromIndex(index);
	if (!item)
		return;

	SubscriptionLastMsgModel::LastMsgItem lastMsg = SubscriptionLastMsgModel::modelItem2LastMsg(*item);

	painter->save();
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);

	QFont originalFont = painter->font();
	QFont smallFont = originalFont;
	smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/4);
	QFont bigFont = originalFont;
	bigFont.setPointSizeF(originalFont.pointSizeF() + 0.5);

	// draw border
	QColor borderColor("#f0f0f0");
	painter->setPen(borderColor);
	// draw left line
	painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
	painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
	if (index.row() == 0)
		painter->drawLine(option.rect.topLeft(), option.rect.topRight());
	painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

	// draw background
	QColor selectedBg("#fff5cc");
	QColor hoveredBg("#dfeefa");
	QRect paintRect = option.rect;
	paintRect.adjust(1, 1, -1, -1);
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, selectedBg);
	}
	else if(option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(paintRect, hoveredBg);
	}

	QString id = lastMsg.subId;
	QPixmap logo = qPmApp->getModelManager()->subscriptionLogo(id);
	QString name = lastMsg.subName;
	QString lastTime = lastMsg.lastTime;
	QString lastBody = lastMsg.lastBody;

	// draw logo
	logo = logo.scaled(QSize(42, 42), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPoint ptLogo = paintRect.topLeft();
	ptLogo.rx() += 10;
	ptLogo.ry() += (paintRect.height()-logo.height())/2;
	painter->drawPixmap(ptLogo, logo);

	// computer right rect
	QRect rectRight = paintRect;
	rectRight.setLeft(rectRight.left() + 10 + logo.width() + 10);
	rectRight.setRight(rectRight.right() - 10);
	rectRight.setTop(rectRight.top() + (rectRight.height()-logo.height())/2);
	rectRight.setHeight(logo.height());

	int unreadMsgCount = qPmApp->getSubscriptionMsgManager()->unreadMsgCount(id);
	if (unreadMsgCount > 0)
	{
		// unread count
		QRect msgCountRect(ptLogo, logo.size());
		msgCountRect.setLeft(msgCountRect.right() - 10);
		msgCountRect.setTop(msgCountRect.top() - 7);
		msgCountRect.setHeight(14);
		msgCountRect.setWidth(20);

		// draw background
		QPixmap unreadBack(":/images/unread_back.png");
		painter->drawPixmap(msgCountRect.topLeft(), unreadBack);

		// draw count
		painter->setFont(smallFont);
		painter->setBrush(Qt::NoBrush);
		painter->setPen(Qt::white);
		painter->setFont(smallFont);
		QString unreadMsgCountText = QString::number(unreadMsgCount);
		if (unreadMsgCount > 99)
			unreadMsgCountText = QString("99+");
		painter->drawText(msgCountRect, Qt::AlignHCenter|Qt::AlignVCenter, unreadMsgCountText);
	}

	// draw time
	QDateTime msgDateTime = CDateTime::localDateTimeFromUtcString(lastTime);
	QDateTime nowDateTime = CDateTime::currentDateTime();
	QString timeText;
	int days = msgDateTime.daysTo(nowDateTime);
	if (days == 0)
		timeText = msgDateTime.toString("hh:mm");
	else if (days == 1)
		timeText = tr("yesterday");
	else
		timeText = msgDateTime.toString("M-d");
	painter->setFont(smallFont);
	painter->setPen(QColor(136, 136, 136));
	QRect timeRect = rectRight;
	timeRect.setHeight(timeRect.height()/2);
	painter->drawText(timeRect, Qt::AlignRight|Qt::AlignVCenter, timeText);
	int timeWidth = QFontMetrics(smallFont).width(timeText);

	// draw last body
	QRect lastBodyRect = rectRight;
	lastBodyRect.setTop(lastBodyRect.top() + rectRight.height()/2);
	painter->setFont(originalFont);
	painter->setPen(QColor(136, 136, 136));
	lastBody = QFontMetrics(originalFont).elidedText(lastBody, Qt::ElideRight, lastBodyRect.width());
	painter->drawText(lastBodyRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, lastBody);

	// draw name
	QRect nameRect = rectRight;
	nameRect.setHeight(nameRect.height()/2);
	if (timeWidth > 0)
		nameRect.setRight(nameRect.right() - timeWidth - 10);
	name = QFontMetrics(bigFont).elidedText(name, Qt::ElideRight, nameRect.width());
	painter->setFont(bigFont);
	painter->setPen(QColor(0, 0, 0));
	painter->drawText(nameRect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, name);

	painter->restore();
}
