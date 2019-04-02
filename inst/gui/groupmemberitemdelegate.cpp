#include "groupmemberitemdelegate.h"
#include "model/groupitemlistmodeldef.h"
#include "model/groupmemberitemdef.h"
#include <QPainter>
#include "PmApp.h"
#include "model/ModelManager.h"
#include <QBitmap>

GroupMemberItemDelegate::GroupMemberItemDelegate(QObject *parent)
: QItemDelegate(parent), m_groupMemberModel(0)
{
}

QSize GroupMemberItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(26, 26);
}

void GroupMemberItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (m_groupMemberModel == 0)
		return;

	GroupMemberItem *memberItem = m_groupMemberModel->nodeFromProxyIndex(index);
	if (!memberItem)
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
	QString id = memberItem->itemId();
	QPixmap avatar = modelManager->getUserAvatar(id);
	QSize avatarSize(20, 20);
	QRect rect = paintRect;
	rect.setWidth(avatarSize.width());
	rect.setHeight(avatarSize.height());
	rect.moveTop(rect.top() + (paintRect.height() - avatarSize.height())/2);
	rect.moveLeft(rect.left() + 9);
	avatar = avatar.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QBitmap avatarMask(":/images/Icon_60_mask20.png");
	avatar.setMask(avatarMask);
	painter->drawPixmap(rect.topLeft(), avatar);

	// draw owner flag
	bool owner = (memberItem->memberRole() == GroupMemberItem::MemberOwner);
	int rightGap = 2;
	if (owner)
	{
		QPixmap ownerPixmap(":/images/Icon_132.png");
		rect = paintRect;
		rect.setTop(rect.top() + (paintRect.height() - ownerPixmap.height())/2);
		rect.setRight(rect.right() - rightGap);
		rect.setLeft(rect.right() - ownerPixmap.width());
		painter->drawPixmap(rect.topLeft(), ownerPixmap);
		rightGap += (ownerPixmap.width() + 2);
	}

	// draw name
	rect = paintRect;
	rect.setLeft(rect.left() + 9 + avatarSize.width() + 6);
	rect.setTop(rect.top() + (paintRect.height() - avatarSize.height())/2);
	rect.setRight(rect.right() - rightGap);
	QColor nameTextColor = QColor(0, 0, 0);
	painter->setPen(nameTextColor);
	QString text = m_groupMemberModel->memberNameInGroup(id);
	text = option.fontMetrics.elidedText(text, Qt::ElideRight, rect.width());
	painter->drawText(rect, text);

	painter->restore();
}
