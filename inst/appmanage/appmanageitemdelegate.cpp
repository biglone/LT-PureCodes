#include "appmanageitemdelegate.h"
#include <QPainter>
#include <QModelIndex>
#include <QPixmap>
#include <QTableView>
#include "appmanagemodel.h"
#include <QFileIconProvider>

AppManageItemDelegate::AppManageItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{

}

AppManageItemDelegate::~AppManageItemDelegate()
{

}

QSize AppManageItemDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/) const
{
	return QSize(112, 135);
}

void AppManageItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QTableView *tableView = (QTableView *)(this->parent());
	if (!tableView)
		return;

	if (!index.isValid())
		return;

	QString path = index.data(AppManageModel::kPathRole).toString();
	if (path.isEmpty())
		return;

	const QStandardItemModel *model = qobject_cast<const QStandardItemModel *>(index.model());
	if (!model)
		return;

	tableView->setColumnWidth(index.column(), 112);
	tableView->setRowHeight(index.row(), 135);

	painter->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);

	QRect rect = option.rect;
	// draw background
	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::NoBrush);
	QPoint pt(rect.left()+25, rect.top()+25);
	QPixmap bgPixmap(":/images/Icon_128.png");

	int actualData = index.data(AppManageModel::kActualDataRole).toInt();
	if (actualData != AppManageModel::Del)
	{
		if (option.state & QStyle::State_MouseOver)
		{
			bgPixmap = QPixmap(":/images/Icon_128_hover.png");
			painter->drawPixmap(pt, bgPixmap);
		}
		else
		{
			painter->drawPixmap(pt, bgPixmap);
		}
	}

	if (actualData == AppManageModel::Data)
	{
		QStandardItem *item = model->itemFromIndex(index);
		QIcon icon = item->icon();
		QString name = item->text();

		// draw icon
		QPixmap iconPixmap = icon.pixmap(48);
		pt.setX(pt.x() + (bgPixmap.width() - iconPixmap.width())/2);
		pt.setY(pt.y() + (bgPixmap.height() - iconPixmap.height())/2);
		painter->drawPixmap(pt, iconPixmap);

		// draw name
		QRect nameRect = rect;
		nameRect.setTop(rect.top() + 25 + bgPixmap.height());
		painter->setPen(Qt::black);
		name = option.fontMetrics.elidedText(name, Qt::ElideRight, nameRect.width()-12);
		painter->drawText(nameRect, Qt::AlignCenter, name);
	}
	else if (actualData == AppManageModel::Add || actualData == AppManageModel::Del)
	{
		QPixmap signPixmap;
		QString signName;
		if (actualData == AppManageModel::Add)
		{
			signPixmap = QPixmap(":/images/Icon_125.png");
			if (option.state & QStyle::State_MouseOver)
				signPixmap = QPixmap(":/images/Icon_125_hover.png");
			signName = tr("Add");
		}
		else
		{
			QFileIconProvider iconProvider;
			signPixmap = iconProvider.icon(QFileIconProvider::Trashcan).pixmap(48);
			signName = tr("Remove");
		}

		// draw sign
		pt.setX(pt.x() + (bgPixmap.width() - signPixmap.width())/2);
		pt.setY(pt.y() + (bgPixmap.height() - signPixmap.height())/2);
		painter->drawPixmap(pt, signPixmap);

		// draw name
		QRect nameRect = rect;
		nameRect.setTop(rect.top() + 25 + bgPixmap.height());
		painter->setPen(Qt::black);
		painter->drawText(nameRect, Qt::AlignCenter, signName);
	}

	painter->setPen(QColor(229, 229, 229)); // color to draw drop indicator
}
