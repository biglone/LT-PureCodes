#include <QDebug>
#include "PmApp.h"
#include <QPainter>
#include <QPixmap>
#include "model/ModelManager.h"
#include "RosterDelegate.h"
#include <QTreeView>
#include "model/rostermodeldef.h"
#include <QDebug>
#include "bean/DetailItem.h"
#include "flickerhelper.h"
#include "PmApp.h"
#include "manager/presencemanager.h"
#include "util/PinYinConverter.h"
#include "Account.h"

static const QSize kBigAvatarSize = QSize(42, 42);
static const QSize kSmallAvatarSize = QSize(24, 24);

RosterDelegate::RosterDelegate(QObject *parent) : QItemDelegate(parent), m_flickerHelper(0), m_avatarType(BigAvatar)
{
	m_avatarMask = QBitmap(":/images/Icon_60_mask42.png");
	m_rosterModel = 0;
}

QSize RosterDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
	RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(index.data(RosterModel::TypeRole).toInt());
	if (itemType == RosterModel::Group)
	{
		return QSize(22, 22);
	}
	else
	{
		if (m_avatarType == BigAvatar)
			return QSize(58, 58);
		else
			return QSize(36, 36);
	}
}

void RosterDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (m_rosterModel == 0)
		return;

	QTreeView *view = static_cast<QTreeView *>(parent());
	if (!view)
		return;

	painter->save();
	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);
	QRect paintRect = option.rect;
	QModelIndex sourceIndex = m_rosterModel->proxyModel()->mapToSource(index);
	RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(sourceIndex.data(RosterModel::TypeRole).toInt());
	if (itemType == RosterModel::Group)
	{
		QStandardItem *groupItem = m_rosterModel->nodeFromProxyIndex(index);
		if (!view->isExpanded(index) && isGroupFlickering(groupItem))
		{
			if (m_flickerHelper && (m_flickerHelper->flickerIndex()%3) == 0)
				drawGroupItem(painter, option, index);
		}
		else
		{
			drawGroupItem(painter, option, index);
		}
	}
	else
	{
		if (m_avatarType == BigAvatar)
			drawBigAvatarRosterItem(painter, option, index);
		else
			drawSmallAvatarRosterItem(painter, option, index);
	}

	painter->restore();
}

void RosterDelegate::drawGroupItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(index.data(RosterModel::TypeRole).toInt());
	if (itemType != RosterModel::Group)
		return;

	QStandardItem *groupItem = m_rosterModel->nodeFromProxyIndex(index);
	QTreeView *view = static_cast<QTreeView*>(parent());

	if (!groupItem || !view)
		return;

	// fill item background color
	QRect paintRect = option.rect;
	paintRect.setHeight(paintRect.height() - 1);
	painter->fillRect(paintRect, QColor("#f7f7f7"));

	// draw name
	painter->save();
	QFont font = painter->font();
	font.setPointSize(9);
	painter->setFont(font);

	QString groupName = groupItem->text();
	QString drawGroupName = groupName;

	QFontMetrics fm = option.fontMetrics;
	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + 5);
	paintRect.setRight(paintRect.right() - 68);
	QString groupText = fm.elidedText(drawGroupName, Qt::ElideMiddle, paintRect.width());
	painter->setPen(QColor("#808080"));
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, groupText);

	/*
	// draw count text
	QString groupCountText = QString("[%1/%2]")
		.arg(m_rosterModel->availableRosterCount(groupName))
		.arg(m_rosterModel->rosterCount(groupName));
	paintRect = option.rect;
	paintRect.setRight(paintRect.right() - 5);
	painter->drawText(paintRect, Qt::AlignRight|Qt::AlignVCenter, groupCountText);
	*/

	painter->restore();
}

void RosterDelegate::drawBigAvatarRosterItem(QPainter* painter, 
											 const QStyleOptionViewItem& option, 
											 const QModelIndex& index) const
{
	RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(index.data(RosterModel::TypeRole).toInt());
	if (itemType != RosterModel::Roster)
		return;

	QStandardItem *rosterItem = m_rosterModel->nodeFromProxyIndex(index);
	if (!rosterItem)
		return;

	QString id = rosterItem->data(RosterModel::IdRole).toString();
	if (id.isEmpty())
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	bean::DetailItem *detail = modelManager->detailItem(id); 
	if (!detail)
		return;

	// fill item background color
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
	
	QModelIndex nextIndex = index.sibling(index.row()+1, index.column());
	if (nextIndex.isValid())
	{
		QStandardItem *nextItem = m_rosterModel->nodeFromProxyIndex(nextIndex);
		if (nextItem && nextItem->data(RosterModel::TypeRole).toInt() == RosterModel::Roster)
		{
			QPoint leftPt = option.rect.bottomLeft();
			leftPt.rx() += 10;
			QPoint rightPt = option.rect.bottomRight();
			rightPt.rx() -= 10;
			painter->setPen(QColor("#f0f0f0"));
			painter->drawLine(leftPt, rightPt);
		}
	}
	
	// draw avatar
	QPixmap avatar = modelManager->getUserAvatar(id);
	QSize avatarSize(kBigAvatarSize);
	QRect rect = paintRect;
	rect.setWidth(avatarSize.width());
	rect.setHeight(avatarSize.height());
	rect.moveTop(rect.y() + (paintRect.height() - avatarSize.height())/2);
	rect.moveLeft(rect.x() + 5);
	if (isContactFlickering(rosterItem))
	{
		int fIndex = m_flickerHelper->flickerIndex();
		if ((fIndex%4) == 1)
			rect.translate(QPoint(2, 2));
		else if ((fIndex%4) == 3)
			rect.translate(QPoint(-2, 2));
	}
	avatar = avatar.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	avatar.setMask(m_avatarMask);
	painter->drawPixmap(rect.topLeft(), avatar);
	
	// draw name
	QFont orignalFont = painter->font();
	QFont newFont(orignalFont);
	newFont.setPointSizeF(orignalFont.pointSizeF() + 0.5);
	painter->setFont(newFont);

	QColor nameTextColor(0, 0, 0);
	QColor nameTextDisabledColor(128, 128, 128);
	rect = paintRect;
	rect.setLeft(rect.left() + 5 + avatarSize.width() + 5);
	rect.setRight(rect.right() - 30);
	rect.setTop(rect.top() + 3);
	rect.setBottom(rect.bottom() - 3);
	rect.setHeight(rect.height()/2);
	painter->setPen(nameTextColor);
	QString name = rosterItem->text();
	if (detail->isDisabled())
	{
		name.append(tr("(invalid)"));
		painter->setPen(nameTextDisabledColor);
	}
	name = QFontMetrics(newFont).elidedText(name, Qt::ElideMiddle, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, name);
	
	// draw terminal icon
	QPixmap terminalPixmap = modelManager->getTerminalIcon(detail);
	if (!terminalPixmap.isNull())
	{
		QPoint ptTerminal = rect.topRight();
		ptTerminal.setX(paintRect.right() - 25);
		ptTerminal.setY(ptTerminal.y() + (rect.height() - terminalPixmap.height())/2);
		painter->drawPixmap(ptTerminal, terminalPixmap);
	}

	rect.moveTop(rect.top() + rect.height());
	rect.setRight(paintRect.right() - 5);

	// draw signature
	painter->setFont(orignalFont);
	QColor signatureTextColor(136, 136, 136);
	QString signature = modelManager->getSignature(detail);
	if (!signature.isEmpty())
	{
		painter->setPen(signatureTextColor);
		signature = option.fontMetrics.elidedText(signature, Qt::ElideRight, rect.width());
		painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, signature);
	}
}

void RosterDelegate::drawSmallAvatarRosterItem(QPainter* painter, 
											   const QStyleOptionViewItem& option, 
											   const QModelIndex& index) const
{
	RosterModel::ItemType itemType = static_cast<RosterModel::ItemType>(index.data(RosterModel::TypeRole).toInt());
	if (itemType != RosterModel::Roster)
		return;

	QStandardItem *rosterItem = m_rosterModel->nodeFromProxyIndex(index);
	if (!rosterItem)
		return;

	ModelManager *modelManager = qPmApp->getModelManager();
	QString id = rosterItem->data(RosterModel::IdRole).toString();
	bean::DetailItem *detail = modelManager->detailItem(id); 
	QFont orignalFont = painter->font();

	// fill item background color
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

	QModelIndex nextIndex = index.sibling(index.row()+1, index.column());
	if (nextIndex.isValid())
	{
		QStandardItem *nextItem = m_rosterModel->nodeFromProxyIndex(nextIndex);
		if (nextItem && nextItem->data(RosterModel::TypeRole).toInt() == RosterModel::Roster)
		{
			QPoint leftPt = option.rect.bottomLeft();
			leftPt.rx() += 10;
			QPoint rightPt = option.rect.bottomRight();
			rightPt.rx() -= 10;
			painter->setPen(QColor("#f0f0f0"));
			painter->drawLine(leftPt, rightPt);
		}
	}

	// draw avatar
	QPixmap avatar = modelManager->getUserAvatar(id);
	QSize avatarSize(kSmallAvatarSize);
	QRect rect = paintRect;
	rect.setWidth(avatarSize.width());
	rect.setHeight(avatarSize.height());
	rect.moveTop(rect.y() + (paintRect.height() - avatarSize.height())/2);
	rect.moveLeft(rect.x() + 16);
	if (isContactFlickering(rosterItem))
	{
		int fIndex = m_flickerHelper->flickerIndex();
		if ((fIndex%4) == 1)
			rect.translate(QPoint(2, 2));
		else if ((fIndex%4) == 3)
			rect.translate(QPoint(-2, 2));
	}
	avatar = avatar.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	avatar.setMask(m_avatarMask);
	painter->drawPixmap(rect.topLeft(), avatar);

	// draw name
	QColor nameTextColor(0, 0, 0);
	QColor nameTextDisabledColor(128, 128, 128);
	rect = paintRect;
	rect.setLeft(rect.left() + 16 + avatarSize.width() + 5);
	rect.setRight(rect.right() - 30);
	rect.setTop(rect.top() + 3);
	rect.setBottom(rect.bottom() - 3);
	painter->setPen(nameTextColor);
	QString name = rosterItem->text();
	if (detail->isDisabled())
	{
		name.append(tr("(invalid)"));
		painter->setPen(nameTextDisabledColor);
	}
	name = QFontMetrics(painter->font()).elidedText(name, Qt::ElideRight, rect.width());
	painter->drawText(rect, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine, name);

	// draw terminal icon
	QPixmap terminalPixmap = modelManager->getTerminalIcon(detail);
	if (!terminalPixmap.isNull())
	{
		QPoint ptTerminal = rect.topRight();
		ptTerminal.setX(paintRect.right() - 25);
		ptTerminal.setY(ptTerminal.y() + (rect.height() - terminalPixmap.height())/2);
		painter->drawPixmap(ptTerminal, terminalPixmap);
	}

	painter->setFont(orignalFont);
}

bool RosterDelegate::isGroupFlickering(QStandardItem *groupItem) const
{
	if (!m_flickerHelper)
		return false;

	QString groupName = groupItem->text();
	QStringList ids = m_rosterModel->groupRosters(groupName);
	foreach (QString id, ids)
	{
		if (m_flickerHelper->containsFlickerItem(id, bean::Message_Chat))
			return true;
	}
	return false;
}

bool RosterDelegate::isContactFlickering(QStandardItem *contactItem) const
{
	if (!m_flickerHelper)
		return false;

	QString id = contactItem->data(RosterModel::IdRole).toString();
	if (m_flickerHelper->containsFlickerItem(id, bean::Message_Chat))
		return true;
	else
		return false;
}