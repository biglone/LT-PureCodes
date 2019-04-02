#include "globalnotificationitemdelegate.h"
#include "PmApp.h"
#include <QPainter>
#include "globalnotificationmodel.h"
#include <QTableView>
#include "globalnotificationmsgmanager.h"

static const int kCellWidth  = 82;
static const int kCellHeight = 108;

GlobalNotificationItemDelegate::GlobalNotificationItemDelegate(GlobalNotificationModel *model, QObject *parent)
	: QItemDelegate(parent), m_model(model)
{
	Q_ASSERT(m_model);
}

GlobalNotificationItemDelegate::~GlobalNotificationItemDelegate()
{

}

QSize GlobalNotificationItemDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
	return QSize(kCellWidth, kCellHeight);
}

void GlobalNotificationItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QTableView *tableView = (QTableView *)(this->parent());
	if (!tableView)
		return;

	if (!index.isValid())
		return;

	tableView->setColumnWidth(index.column(), kCellWidth);
	tableView->setRowHeight(index.row(), kCellHeight);

	QStandardItem *item = m_model->itemFromIndex(index);
	if (!item)
		return;

	GlobalNotificationDetail globalNotification = GlobalNotificationModel::modelItem2GlobalNotification(*item);
	if (!globalNotification.isValid())
		return;

	painter->save();
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);

	QRect rect = option.rect;
	QColor borderColor(227, 227, 227);
	QRect backRect = option.rect;
	backRect.adjust(3, 1, -3, -1);
	if ((option.state & QStyle::State_Selected) || (option.state & QStyle::State_MouseOver))
	{
		painter->fillRect(backRect, borderColor);
	}

	QPixmap logo = m_model->globalNotificationLogo(globalNotification.id());
	QString name = globalNotification.name();
	// QString introduction = subscription.introduction();

	// draw logo
	logo = logo.scaled(QSize(64, 64), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPoint ptLogo = backRect.topLeft();
	ptLogo.rx() += (backRect.width()-logo.width())/2;
	ptLogo.ry() += 9;
	painter->drawPixmap(ptLogo, logo);

	/*
	// draw unread messages
	int unreadMsgCount = qPmApp->getSubscriptionMsgManager()->unreadMsgCount(subscription.id());
	if (unreadMsgCount > 0)
	{
		QRect msgCountRect = rect;
		msgCountRect.setLeft(msgCountRect.left() + 50);
		msgCountRect.setTop(msgCountRect.top() + 6);
		msgCountRect.setHeight(14);
		msgCountRect.setWidth(20);

		// draw background
		QPixmap unreadBack(":/images/unread_back.png");
		painter->drawPixmap(msgCountRect.topLeft(), unreadBack);

		// draw count
		QFont originalFont = painter->font();
		QFont smallFont = originalFont;
		smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/4);
		painter->setFont(smallFont);
		painter->setBrush(Qt::NoBrush);
		painter->setPen(Qt::white);
		painter->setFont(smallFont);
		QString unreadMsgCountText = QString::number(unreadMsgCount);
		if (unreadMsgCount > 99)
			unreadMsgCountText = QString("99+");
		painter->drawText(msgCountRect, Qt::AlignHCenter|Qt::AlignVCenter, unreadMsgCountText);
		painter->setFont(originalFont);
	}
	*/

	// draw name
	/*
	QFont font = painter->font();
	font.setPointSize(font.pointSize()+1);
	painter->setFont(font);
	painter->setPen(Qt::black);
	
	QRect rectRight = rect;
	rectRight.setLeft(rectRight.left() + 66);
	rectRight.setRight(rectRight.right() - 12);
	rectRight.setTop(rectRight.top() + 15);
	rectRight.setHeight(24);
	painter->drawText(rectRight, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, name);

	// draw introduction
	rectRight.translate(0, 24);
	font.setPointSize(font.pointSize()-1);
	painter->setFont(font);
	painter->setPen(QColor(128, 128, 128));
	QFontMetrics fm = painter->fontMetrics();
	introduction = fm.elidedText(introduction, Qt::ElideRight, rectRight.width());
	painter->drawText(rectRight, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, introduction);
	*/

	QRect rectName = rect;
	rectName.setLeft(rectName.left() + 4);
	rectName.setRight(rectName.right() - 4);
	rectName.setTop(rectName.top() + 9 + logo.height());
	name = option.fontMetrics.elidedText(name, Qt::ElideRight, rectName.width());
	painter->setPen(Qt::black);
	painter->drawText(rectName, Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextSingleLine, name);

	painter->restore();
}
