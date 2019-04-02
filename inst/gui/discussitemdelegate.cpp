#include "discussitemdelegate.h"
#include "discussmodeldef.h"
#include "discussitemdef.h"
#include <QPainter>
#include "flickerhelper.h"
#include "login/Account.h"
#include "PmApp.h"
#include "ModelManager.h"
#include "groupitemlistmodeldef.h"
#include <QBitmap>

DiscussItemDelegate::DiscussItemDelegate(QObject *parent)
: QItemDelegate(parent), m_discussModel(0), m_flickerHelper(0)
{
}

QSize DiscussItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
	if (index.data(RosterBaseItem::RosterTypeRole).toInt() == RosterBaseItem::RosterTypeGroup)
		return QSize(22, 22);
	else
		return QSize(30, 30);
}

void DiscussItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (!m_discussModel)
		return;

	if (index.data(RosterBaseItem::RosterTypeRole).toInt() == RosterBaseItem::RosterTypeGroup)
	{
		QString groupId = index.data(RosterBaseItem::RosterIdRole).toString();
		DiscussGroupItem *groupItem = 0;
		if (groupId == DiscussModel::createdGroupId())
			groupItem = m_discussModel->createdGroupItem();
		else
			groupItem = m_discussModel->addedGroupItem();
		QString text = groupItem->itemName();
		text += QString("(%1)").arg(groupItem->rowCount());

		painter->save();
		painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);
		
		// background
		painter->fillRect(option.rect, QColor("#f7f7f7"));

		// draw name
		QFont font = painter->font();
		font.setPointSize(9);
		painter->setFont(font);
		QRect rect = option.rect;
		rect.setLeft(rect.left() + 5);
		painter->setPen(QColor("#808080"));
		painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter, text);
		painter->restore();
	}
	else
	{
		DiscussItem *discussItem = m_discussModel->nodeFromProxyIndex(index);
		if (!discussItem)
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
		QSize avatarSize(20, 20);
		QPixmap avatar(":/images/Icon_64_small.png");
		QRect rect = paintRect;
		rect.setWidth(avatarSize.width());
		rect.setHeight(avatarSize.height());
		rect.moveTop(rect.y() + (paintRect.height() - avatarSize.height())/2);
		rect.moveLeft(rect.x() + 16);
		if (m_flickerHelper)
		{
			if (m_flickerHelper->containsFlickerItem(discussItem->itemId(), bean::Message_DiscussChat))
			{
				int fIndex = m_flickerHelper->flickerIndex();
				if ((fIndex%4) == 1)
					rect.translate(QPoint(2, 2));
				else if ((fIndex%4) == 3)
					rect.translate(QPoint(-2, 2));
			}
		}
		QBitmap avatarMask(":/images/Icon_60_mask20.png");
		avatar.setMask(avatarMask);
		painter->drawPixmap(rect.topLeft(), avatar);

		// draw message setting status
		AccountSettings::GroupMsgSettingType groupSetting = Account::settings()->discussMsgSetting(discussItem->itemId());
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
		rect.setLeft(rect.left() + 16 + avatarSize.width() + 5);
		rect.setTop(rect.top() + (paintRect.height() - avatarSize.height())/2);
		rect.setBottom(rect.bottom() - (paintRect.height() - avatarSize.height())/2);
		rect.setRight(rect.right() - 42);
		QColor nameTextColor = QColor(0, 0, 0);
		painter->setPen(nameTextColor);
		QString text = discussItem->itemName();
		text = option.fontMetrics.elidedText(text, Qt::ElideRight, rect.width());
		painter->drawText(rect, text);

		// draw member count
		GroupItemListModel *memberModel = qPmApp->getModelManager()->discussItemsModel(discussItem->itemId());
		if (memberModel)
		{
			int nameLen = option.fontMetrics.width(text);
			rect.setLeft(rect.left() + nameLen);
			rect.setRight(paintRect.right() - 2);
			int memberCount = memberModel->memberCount();
			if (memberCount > 0)
			{
				QString countText = tr("(%1 members)").arg(memberCount);
				QColor countTextColor(148, 148, 148);
				painter->setPen(countTextColor);
				painter->drawText(rect, countText);
			}
		}

		painter->restore();
	}
}