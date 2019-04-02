#include <QPainter>
#include <QTreeView>
#include "ModelManager.h"
#include "PmApp.h"
#include "model/rostermodeldef.h"
#include "model/rosteritemdef.h"
#include "model/groupitemdef.h"
#include "model/orgstructitemdef.h"
#include "model/orgstructmodeldef.h"
#include "editfilteritemdelegate.h"

EditFilterItemDelegate::EditFilterItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{
}

QSize EditFilterItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
	return QSize(35, 35);
}

void EditFilterItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (!index.isValid())
		return;

	QModelIndex sourceIndex = index;
	RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(sourceIndex.data(RosterBaseItem::RosterTypeRole).toInt());
	if (itemType == RosterBaseItem::RosterTypeGroup)
	{
		drawGroup(painter, option, sourceIndex);
	}
	else
	{
		drawItem(painter, option, sourceIndex);
	}
}

void EditFilterItemDelegate::drawGroup(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(painter);

	QTreeView *view = static_cast<QTreeView *>(parent());
	if (!view)
	{
		return;
	}

	QModelIndex sourceIndex = index;
	QWidget *w = view->indexWidget(sourceIndex);
	if (w)
	{
		w->setGeometry(QRect(option.rect));
	}
}

void EditFilterItemDelegate::drawItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();

	QColor textColor;

	// draw background
	QRect paintRect = option.rect;
	if (option.state & QStyle::State_Selected)
	{
		painter->fillRect(paintRect, QColor("#FFF5CC"));
		textColor = QColor("#000000");
	}
	else if (option.state & QStyle::State_MouseOver)
	{
		painter->fillRect(paintRect, QColor("#DFEEFA"));
		textColor = QColor("#000000");
	}
	else
	{
		painter->fillRect(paintRect, QColor("#FFFFFF"));
		textColor = QColor("#000000");
	}

	RosterBaseItem::RosterItemType itemType = static_cast<RosterBaseItem::RosterItemType>(index.data(RosterBaseItem::RosterTypeRole).toInt());
	QString id = index.data(RosterBaseItem::RosterIdRole).toString();
	ModelManager *modelManager = qPmApp->getModelManager();

	// draw avatar
	paintRect = option.rect;
	QSize iconSize(24, 24);
	QPixmap pixmapAvatar;
	if (itemType == RosterBaseItem::RosterTypeContact)
	{
		pixmapAvatar = modelManager->getUserAvatar(id);
	}
	else if (itemType == RosterBaseItem::RosterTypeGroupMuc)
	{
		pixmapAvatar = modelManager->getGroupLogo(id);
	}
	else if (itemType == RosterBaseItem::RosterTypeDiscuss)
	{
		pixmapAvatar = QPixmap(":/images/Icon_64_middle.png");
	}
	else if (itemType == RosterBaseItem::RosterTypeSubscription)
	{
		pixmapAvatar = modelManager->subscriptionLogo(id);
	}
	pixmapAvatar = pixmapAvatar.scaled(iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	paintRect.moveTop(paintRect.y() + (paintRect.height() - iconSize.height())/2);
	paintRect.moveLeft(paintRect.left() + 4);
	paintRect.setWidth(iconSize.width());
	paintRect.setHeight(iconSize.height());
	painter->drawPixmap(paintRect, pixmapAvatar);

	QFontMetrics fontMetrics = option.fontMetrics;
	painter->setPen(textColor);

	// draw department
	QString dept = index.data(RosterBaseItem::RosterExtraInfoRole).toString();
	QString wid = index.data(OrgStructContactItem::OrgStructWidRole).toString();
	QString tailText;
	if (!wid.isEmpty())
	{
		OrgStructModel *osModel = qPmApp->getModelManager()->orgStructModel();
		OrgStructContactItem *osContact = osModel->contactByWid(wid);
		if (osContact)
		{
			if (osContact->itemUserState() == OrgStructContactItem::USER_STATE_INACTIVE)
				tailText = tr("(unregistered)");
		}
	}
	int deptLen = 0;
	if (!dept.isEmpty())
	{
		paintRect = option.rect;
		paintRect.adjust(4, 0, -4, 0);
		int maxDeptLen = paintRect.width()/2 - 4;
		deptLen = fontMetrics.width(dept);
		QString drawDept = dept;
		if (deptLen > maxDeptLen)
		{
			drawDept = fontMetrics.elidedText(dept, Qt::ElideRight, maxDeptLen);
			deptLen = maxDeptLen;
		}
		paintRect.setLeft(paintRect.right() - deptLen);
		painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, drawDept);
	}

	// draw text
	QString filterName = index.data(RosterBaseItem::RosterExtraInfoRole2).toString();

	paintRect = option.rect;
	paintRect.setLeft(paintRect.left() + 4 + iconSize.width() + 4);
	if (deptLen > 0)
		paintRect.setRight(paintRect.right() - 4 - deptLen - 4);
	else
		paintRect.setRight(paintRect.right() - 4);

	QString text = index.data(Qt::DisplayRole).toString();
	text += tailText;
	if (!tailText.isEmpty())
	{
		painter->setPen(QColor(136, 136, 136));
	}

	QString first = "";
	QString red = "";
	QString end = text;

	int begin = text.indexOf(filterName);
	if (begin >= 0)
	{
		first = text.mid(0, begin);
		red = filterName;
		end = text.mid(begin+filterName.length());
	}

	if (!first.isEmpty())
	{
		first = fontMetrics.elidedText(first, Qt::ElideRight, paintRect.width());
		int len = fontMetrics.width(first);
		painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, first);
		paintRect.setLeft(paintRect.left() + len);
	}

	if (!red.isEmpty())
	{
		QPen oldPen = painter->pen();
		textColor = QColor("#FF0000"); // red
		painter->setPen(textColor);

		red = fontMetrics.elidedText(red, Qt::ElideRight, paintRect.width());
		int len = fontMetrics.width(red);
		painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, red);
		paintRect.setLeft(paintRect.left() + len);

		painter->setPen(oldPen);
	}

	if (!end.isEmpty())
	{
		end = fontMetrics.elidedText(end, Qt::ElideRight, paintRect.width());
		painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, end);
	}

	painter->restore();
}