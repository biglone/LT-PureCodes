#include "unreadmessagelistview.h"
#include "model/unreadmsgitem.h"
#include <QPainter>
#include "ModelManager.h"
#include "offlinemsgmanager.h"
#include "PmApp.h"
#include "util/MaskUtil.h"
#include <QHBoxLayout>
#include <QToolButton>

static const QSize kIconSize = QSize(26, 26);

UnreadMessageListView::UnreadMessageListView(QWidget *parent)
	: QListView(parent)
{
	setStyleSheet("QListView{"
		"background:transparent;"
		"border: none;"
		"}");
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setDragEnabled(false);
	setItemDelegate(new UnreadMessageItemDelegate(this));
}

UnreadMessageListView::~UnreadMessageListView()
{

}

UnreadMessageItemDelegate::UnreadMessageItemDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QSize UnreadMessageItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex & /*index*/) const
{
	QSize sz = QSize(option.rect.width(), 30);
	return sz;
}

void UnreadMessageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(painter);

	QListView *view = static_cast<QListView *>(parent());
	if (!view)
	{
		return;
	}

	QWidget *w = view->indexWidget(index);
	if (w)
	{
		w->setGeometry(QRect(option.rect));
	}
}

UnreadMessageItemWidget::UnreadMessageItemWidget(const QModelIndex &dataIndex, QWidget *parent /*= 0*/)
: QWidget(parent), m_isHover(false), m_dataIndex(dataIndex)
{
	Q_ASSERT(m_dataIndex.isValid());

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addStretch(1);
	m_ignoreButton = new QToolButton(this);
	m_ignoreButton->setIcon(QIcon(":/images/Icon_134.png"));
	m_ignoreButton->setToolTip(tr("Ignore"));
	layout->addWidget(m_ignoreButton);
	layout->setContentsMargins(0, 0, 5, 0);

	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);

	m_ignoreButton->setVisible(false);
	connect(m_ignoreButton, SIGNAL(clicked()), this, SLOT(onIgnoreButtonClicked()));
}

void UnreadMessageItemWidget::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e);

	QPainter painter(this);
	QRect rect = this->rect();
	if(m_isHover)
	{
		painter.fillRect(rect, QColor("#dfeefa"));
	}
	else
	{
		painter.fillRect(rect, QColor("#ffffff"));
	}

	QString id = m_dataIndex.data(UnreadMsgItem::IdRole).toString();
	bean::MessageType msgType = (bean::MessageType)m_dataIndex.data(UnreadMsgItem::MsgTypeRole).toInt();
	ModelManager *modelManager = qPmApp->getModelManager();

	// draw avatar
	QVariant value = m_dataIndex.data(Qt::DecorationRole);
	QSize iconSize = kIconSize;
	QRect iconPaintRect = rect;
	iconPaintRect.setLeft(iconPaintRect.left() + 6);
	if(value.type() == QVariant::Icon)
	{
		QIcon icon = qvariant_cast<QIcon>(value);
		QPixmap pixmap = icon.pixmap(iconSize, QIcon::Normal, QIcon::On);
		if (msgType == bean::Message_Chat)
			pixmap = modelManager->getUserAvatar(id);
		else if (msgType == bean::Message_GroupChat)
			pixmap = modelManager->getGroupLogo(id);
		else if (msgType == bean::Message_DiscussChat)
			pixmap = modelManager->discussLogo(id);
		pixmap = pixmap.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		iconPaintRect.moveTop(iconPaintRect.top() + (iconPaintRect.height() - iconSize.height())/2);
		iconPaintRect.setWidth(iconSize.width());
		iconPaintRect.setHeight(iconSize.height());

		QPixmap rawMask(":/images/Icon_60_mask.png");
		WidgetBorder border;
		border.top = border.bottom = border.left = border.right = 4;
		QBitmap mask = MaskUtil::generateMask(rawMask, border, iconSize);
		pixmap.setMask(mask);

		painter.drawPixmap(iconPaintRect, pixmap);
	}

	// draw count
	int countWidth = 20;
	int countHeight = 14;
	if (!m_isHover)
	{
		OfflineMsgManager::FromType fType = OfflineMsgManager::messageType2OfflineFromType(msgType);
		int offlineCount = qPmApp->getOfflineMsgManager()->offlineMsgCount(fType, id);
		int msgCount = m_dataIndex.data(UnreadMsgItem::CountRole).toInt();
		msgCount += offlineCount;
		QString count = QString("%1").arg(msgCount);
		if (msgCount > 99)
		{
			count = QString("99+");
		}

		QRect countRect = rect;
		countRect.setRight(countRect.right() - 6);
		countRect.setLeft(countRect.right() - countWidth);
		countRect.setTop(countRect.top() + (countRect.height() - countHeight)/2);
		countRect.setHeight(countHeight);
		
		// draw count background
		painter.save();
		QPixmap unreadBack(":/images/unread_back.png");
		painter.drawPixmap(countRect.topLeft(), unreadBack);

		// draw number
		QFont originalFont = painter.font();
		QFont smallFont = originalFont;
		smallFont.setPointSize(originalFont.pointSize() - originalFont.pointSize()/4);
		painter.setFont(smallFont);
		painter.setPen(QColor(255, 255, 255));
		painter.setBrush(Qt::NoBrush);
		painter.drawText(countRect, Qt::AlignHCenter|Qt::AlignVCenter, count);
		painter.restore();
	}

	// draw name
	QString name;
	if (msgType == bean::Message_Chat)
		name = modelManager->userName(id);
	else if (msgType == bean::Message_GroupChat)
		name = modelManager->groupName(id);
	else if (msgType == bean::Message_DiscussChat)
		name = modelManager->discussName(id);
	else
		name = m_dataIndex.data(UnreadMsgItem::NameRole).toString();
	QRect nameRect = rect;
	nameRect.setLeft(iconPaintRect.right() + 10);
	nameRect.setRight(rect.right() - 6 - countWidth - 5);
	nameRect.setTop(nameRect.top() + (nameRect.height() - painter.fontMetrics().height())/2);
	nameRect.setHeight(painter.fontMetrics().height());
	name = painter.fontMetrics().elidedText(name, Qt::ElideRight, nameRect.width());
	painter.drawText(nameRect, name);
}

void UnreadMessageItemWidget::enterEvent(QEvent *e)
{
	m_isHover = true;
	m_ignoreButton->setVisible(true);

	QWidget::enterEvent(e);
}

void UnreadMessageItemWidget::leaveEvent(QEvent *e)
{
	QWidget::leaveEvent(e);

	m_isHover = false;
	m_ignoreButton->setVisible(false);
}

void UnreadMessageItemWidget::onIgnoreButtonClicked()
{
	emit ignoreItem(m_dataIndex);
}