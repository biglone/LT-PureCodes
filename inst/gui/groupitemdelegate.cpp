#include "groupitemdelegate.h"
#include "groupmodeldef.h"
#include "groupitemdef.h"
#include <QPainter>
#include "flickerhelper.h"
#include "login/Account.h"
#include "PmApp.h"
#include "ModelManager.h"
#include <QBitmap>

GroupItemDelegate::GroupItemDelegate(QObject *parent)
 : QItemDelegate(parent), m_groupModel(0), m_flickerHelper(0)
{
}

QSize GroupItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(30, 30);
}

void GroupItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (m_groupModel == 0)
		return;

	MucGroupItem *groupItem = m_groupModel->nodeFromProxyIndex(index);
	if (!groupItem)
		return;

	painter->save();
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);
	
	// draw background
	QColor selectedBg("#fff5cc");
	QColor hoveredBg("#dfeefa");
	QRect paintRect = option.rect;
	paintRect.setHeight(paintRect.height() - 1);
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, selectedBg);
	}
	else if(option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(paintRect, hoveredBg);
	}

	// draw avatar
	ModelManager *modelManager = qPmApp->getModelManager();
	QPixmap avatar = modelManager->getGroupLogo(groupItem->itemId());
	QSize avatarSize(20, 20);
	QRect rect = paintRect;
	rect.setWidth(avatarSize.width());
	rect.setHeight(avatarSize.height());
	rect.moveTop(rect.y() + (paintRect.height() - avatarSize.height())/2);
	rect.moveLeft(rect.x() + 5);
	if (m_flickerHelper)
	{
		if (m_flickerHelper->containsFlickerItem(groupItem->itemId(), bean::Message_GroupChat))
		{
			int fIndex = m_flickerHelper->flickerIndex();
			if ((fIndex%4) == 1)
				rect.translate(QPoint(2, 2));
			else if ((fIndex%4) == 3)
				rect.translate(QPoint(-2, 2));
		}
	}
	avatar = avatar.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QBitmap avatarMask(":/images/Icon_60_mask20.png");
	avatar.setMask(avatarMask);
	painter->drawPixmap(rect.topLeft(), avatar);

	// draw message setting status
	AccountSettings::GroupMsgSettingType groupSetting = Account::settings()->groupMsgSetting(groupItem->itemId());
	if (groupSetting == AccountSettings::UnTip)
	{
		QPixmap msgStatus(":/images/Icon_108.png");
		msgStatus = msgStatus.scaled(QSize(10, 10), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		QRect msgStatusRect = rect;
		msgStatusRect.setLeft(rect.right()-msgStatus.width());
		msgStatusRect.setTop(rect.bottom()-msgStatus.height());
		msgStatusRect.translate(3, 3);
		painter->drawPixmap(msgStatusRect.topLeft(), msgStatus);
	}

	// draw name
	rect = paintRect;
	rect.setLeft(rect.left() + 5 + avatarSize.width() + 5);
	rect.setTop(rect.top() + (paintRect.height() - avatarSize.height())/2);
	rect.setBottom(rect.bottom() - (paintRect.height() - avatarSize.height())/2);
	QColor nameTextColor = QColor(0, 0, 0);
	painter->setPen(nameTextColor);
	QString text = groupItem->itemName();
	text = option.fontMetrics.elidedText(text, Qt::ElideRight, rect.width());
	painter->drawText(rect, text);

	painter->restore();
}