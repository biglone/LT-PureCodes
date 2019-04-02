
#include <QDebug>
#include <QTreeView>
#include <QPainter>
#include <QPixmap>
#include "model/orgstructitemdef.h"
#include "model/orgstructmodeldef.h"
#include "flickerhelper.h"
#include "PmApp.h"
#include "model/ModelManager.h"
#include "bean/DetailItem.h"
#include "OrganizationDelegate.h"
#include <QBitmap>

OrganizationDelegate::OrganizationDelegate( QObject *parent /*= 0*/ )
: QItemDelegate(parent)
, m_orgModel(0)
, m_flickerHelper(0)
{
	m_openBranchImg = QImage(":/images/Icon_42.png");
	m_closeBranchImg = QImage(":/images/Icon_43.png");
	m_contactPixmap = QPixmap::fromImage(ModelManager::avatarDefaultSmallIcon());
	QBitmap contactMask(":/images/Icon_60_mask20.png");
	m_contactPixmap.setMask(contactMask);
}

OrganizationDelegate::~OrganizationDelegate()
{

}
QSize OrganizationDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(30, 30);
}

void OrganizationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (m_orgModel == 0)
		return;

	QTreeView *view = static_cast<QTreeView *>(parent());
	if (!view)
		return;

	painter->save();
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);
	QRect paintRect = option.rect;
	RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
	if (itemType == RosterBaseItem::RosterTypeGroup)
	{
		drawGroupItem(painter, option, index);
	}
	else
	{
		drawContactItem(painter, option, index);
	}

	painter->restore();
}

void OrganizationDelegate::drawGroupItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
	if (itemType != RosterBaseItem::RosterTypeGroup)
		return;

	OrgStructGroupItem *groupItem = (OrgStructGroupItem *)m_orgModel->orgStructItemFromIndex(index);
	QTreeView *view = static_cast<QTreeView*>(parent());
	if (!groupItem || !view)
		return;

	// draw icon
	QRect paintRect = option.rect;
	bool expanded = view->isExpanded(index);
	QImage branchImg;
	if (expanded)
	{
		branchImg = m_openBranchImg;
	}
	else
	{
		branchImg = m_closeBranchImg;
	}

	int depth = 0;
	paintRect.setTop(paintRect.top() + (paintRect.height() - branchImg.height())/2);
	paintRect.setLeft(paintRect.left() + kmargin*depth);
	paintRect.setHeight(branchImg.height());
	paintRect.setWidth(branchImg.width());
	paintRect.adjust(0, -2, 0, -2);
	painter->drawImage(paintRect, branchImg);

	// draw name
	painter->save();
	painter->setFont(option.font);

	QFontMetrics fm = option.fontMetrics;
	
	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + kmargin*depth + branchImg.width() + 2);
	paintRect.setRight(paintRect.right() - 6);
	paintRect.setTop(paintRect.top() + (paintRect.height() - (fm.height()))/2);
	paintRect.adjust(0, -2, 0, -2);
	QString groupText;
	if (!groupItem->isLoading())
	{
		painter->setPen(QColor(0, 0, 0));
		groupText = fm.elidedText(groupItem->itemName(), Qt::ElideMiddle, paintRect.width());
	}
	else
	{
		painter->setPen(QColor(128, 128, 128));
		groupText = tr("loading...");
	}
	painter->drawText(paintRect, groupText);

	painter->restore();
}

void OrganizationDelegate::drawContactItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
	if (itemType != RosterBaseItem::RosterTypeContact)
		return;

	OrgStructContactItem *contactItem = (OrgStructContactItem *)m_orgModel->orgStructItemFromIndex(index);
	if (!contactItem)
		return;

	// fill item background color
	QColor selectedBg("#fff5cc");
	QColor hoveredBg("#dfeefa");
	QRect paintRect = option.rect;
	paintRect.setHeight(paintRect.height() - 1);
	QRect backRect = paintRect;
	backRect.setLeft(0);
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(backRect, selectedBg);
	}
	else if(option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(backRect, hoveredBg);
	}

	// draw avatar
	// int depth = contactItem->depth();
	int depth = 0;
	QRect rect = paintRect;
	rect.setWidth(m_contactPixmap.width());
	rect.setHeight(m_contactPixmap.height());
	rect.moveTop(rect.y() + (paintRect.height() - m_contactPixmap.height())/2);
	rect.moveLeft(rect.x() + kmargin*depth + 6);
	if (isContactFlickering(contactItem))
	{
		int fIndex = m_flickerHelper->flickerIndex();
		if ((fIndex%4) == 1)
			rect.translate(QPoint(2, 2));
		else if ((fIndex%4) == 3)
			rect.translate(QPoint(-2, 2));
	}
	
	QPixmap avatarPixmap = m_contactPixmap;
	/* // show different avatar according to gender
	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *item = modelManager->detailItem(contactItem->itemId());
	if (item)
	{
		int sex = item->sex();
		if (sex < 0 && sex > 1)
		{
			sex = 9;
		}
		if (sex == 0)
		{
			// female
			avatarImg = m_femaleContactImg;
		}
		else if (sex == 1)
		{
			// male
			avatarImg = m_contactImg;
		}
		else
		{
			// secret
			avatarImg = m_contactImg;
		}
	}
	*/
	painter->drawPixmap(rect.topLeft(), avatarPixmap);

	// draw name
	QColor nameTextColor(0, 0, 0);
	QColor unregisterColor(136, 136, 136);
	QString name = contactItem->itemName();
	if (contactItem->itemUserState() == OrgStructContactItem::USER_STATE_INACTIVE)
	{
		name += tr("(unregistered)");
		painter->setPen(unregisterColor);
	}
	else
	{
		painter->setPen(nameTextColor);
	}

	QFontMetrics fm = option.fontMetrics;
	rect = paintRect;
	rect.setLeft(rect.left() + kmargin*depth + m_contactPixmap.width() + 6 + 6);
	rect.setTop(rect.top() + (rect.height() - (fm.height()))/2);
	rect.setRight(rect.right() - 6);

	name = fm.elidedText(name, Qt::ElideRight, rect.width());
	painter->drawText(rect, name);
}

bool OrganizationDelegate::isGroupFlickering( OrgStructGroupItem *groupItem ) const
{
	if (!m_flickerHelper)
		return false;

	foreach (QString id, groupItem->allContactUids())
	{
		if (m_flickerHelper->containsFlickerItem(id, bean::Message_Chat))
			return true;
	}
	return false;
}

bool OrganizationDelegate::isContactFlickering( OrgStructContactItem *contactItem ) const
{
	if (!m_flickerHelper)
		return false;

	if (m_flickerHelper->containsFlickerItem(contactItem->itemId(), bean::Message_Chat))
		return true;
	else
		return false;
}