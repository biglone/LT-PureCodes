#include "appmanagemodel.h"
#include <QFileIconProvider>
#include <QMimeData>
#include <QByteArray>
#include <QPainter>

static const char *kAddFlag  = "app_add";
static const char *kDelFlag  = "app_del";

static const char *NOTEPAD_NAME = "notepad.exe";
static const char *MSTSC_NAME   = "mstsc.exe";
static const char *MSPAINT_NAME = "mspaint.exe";
static const char *CALC_NAME    = "calc.exe";

const char *AppManageModel::kMimeType = "application/app.source.index";

AppManageModel::AppManageModel(QObject *parent)
	: QStandardItemModel(parent)
{
	setRowCount(kRowCount);
	setColumnCount(kColCount);
}

AppManageModel::~AppManageModel()
{

}

QIcon AppManageModel::appSmallIcon(const QString &appPath)
{
	QIcon icon;
	if (appPath.isEmpty())
		return icon;

	QFileInfo fi(appPath);
	if (fi.fileName() == QString::fromLatin1(NOTEPAD_NAME))
	{
		icon = QIcon(":/appmanage/notebook_small.png");
	}
	else if (fi.fileName() == QString::fromLatin1(MSTSC_NAME))
	{
		icon = QIcon(":/appmanage/computer_small.png");
	}
	else if (fi.fileName() == QString::fromLatin1(MSPAINT_NAME))
	{
		icon = QIcon(":/appmanage/drawing_small.png");
	}
	else if (fi.fileName() == QString::fromLatin1(CALC_NAME))
	{
		icon = QIcon(":/appmanage/calculator_small.png");
	}
	else
	{
		QFileIconProvider iconProvider;
		icon = iconProvider.icon(appPath);
		if (icon.isNull())
			icon = iconProvider.icon(QFileIconProvider::File);
		QPixmap rawPixmap = icon.pixmap(16);
		QPixmap iconPixmap(20, 20);
		QPainter painter(&iconPixmap);
		painter.setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);
		painter.fillRect(0, 0, 20, 20, Qt::white);
		painter.drawPixmap(QPoint(2, 2), rawPixmap);
		icon = iconPixmap;
	}
	return icon;
}

QIcon AppManageModel::appBigIcon(const QString &appPath)
{
	QIcon icon;
	if (appPath.isEmpty())
		return icon;

	QFileInfo fi(appPath);
	if (fi.fileName() == QString::fromLatin1(NOTEPAD_NAME))
	{
		icon = QIcon(":/appmanage/notebook_big.png");
	}
	else if (fi.fileName() == QString::fromLatin1(MSTSC_NAME))
	{
		icon = QIcon(":/appmanage/computer_big.png");
	}
	else if (fi.fileName() == QString::fromLatin1(MSPAINT_NAME))
	{
		icon = QIcon(":/appmanage/drawing_big.png");
	}
	else if (fi.fileName() == QString::fromLatin1(CALC_NAME))
	{
		icon = QIcon(":/appmanage/calculator_big.png");
	}
	else
	{
		QFileIconProvider iconProvider;
		icon = iconProvider.icon(appPath);
		if (icon.isNull())
			icon = iconProvider.icon(QFileIconProvider::File);
	}
	return icon;
}

void AppManageModel::setAppItem(int index, const QString &name, const QString &path)
{
	if (index >= kRowCount*kColCount-1)
		return;

	QIcon icon = appBigIcon(path);
	QStandardItem *item = new QStandardItem(icon, name);
	item->setData(path, AppManageModel::kPathRole);
	item->setData(AppManageModel::Data, AppManageModel::kActualDataRole);
	item->setToolTip(name);
	int row = index/kColCount;
	int col = index%kColCount;
	setItem(row, col, item);
}

void AppManageModel::setAddItem(int index)
{
	if (index >= kRowCount*kColCount-1)
		return;

	QStandardItem *item = new QStandardItem();
	item->setData(kAddFlag, AppManageModel::kPathRole);
	item->setData(AppManageModel::Add, AppManageModel::kActualDataRole);
	item->setToolTip(tr("Click to add app"));
	int row = index/kColCount;
	int col = index%kColCount;
	setItem(row, col, item);
}

void AppManageModel::setDelItem()
{
	QStandardItem *item = new QStandardItem();
	item->setData(kDelFlag, AppManageModel::kPathRole);
	item->setData(AppManageModel::Del, AppManageModel::kActualDataRole);
	item->setToolTip(tr("Drag app here to remove"));
	setItem(kRowCount-1, kColCount-1, item);
}

bool AppManageModel::mimeDataToIndex(const QByteArray &data, int &row, int &col)
{
	QString text = QString::fromLatin1(data);
	if (text.isEmpty())
		return false;

	QStringList rowAndCol = text.split(" ");
	if (rowAndCol.count() != 2)
		return false;

	row = rowAndCol[0].toInt();
	col = rowAndCol[1].toInt();
	return true;
}

Qt::DropActions AppManageModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

Qt::ItemFlags AppManageModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
	Qt::ItemFlags newFlags = (Qt::ItemFlags)(defaultFlags & (~Qt::ItemIsDragEnabled) & (~Qt::ItemIsDropEnabled));

	if (index.isValid())
	{
		QStandardItem *item = this->item(index.row(), index.column());
		if (!item)
		{
			return newFlags;
		}

		int actualData = index.data(kActualDataRole).toInt();
		if (actualData == AppManageModel::Data)
		{
			return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | newFlags;
		}
		else if (actualData == AppManageModel::Del)
		{
			return Qt::ItemIsDropEnabled | newFlags;
		}
		else
		{
			return newFlags;
		}
	}
	else
	{
		return newFlags;
	}
}

QStringList AppManageModel::mimeTypes() const
{
	QStringList types;
	types << kMimeType;
	return types;
}

QMimeData *AppManageModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QByteArray encodedData;
	foreach (const QModelIndex &index, indexes) {
		if (index.isValid()) {
			QString text = QString("%1 %2").arg(index.row()).arg(index.column());
			encodedData = text.toLatin1();
			break;
		}
	}

	mimeData->setData(kMimeType, encodedData);
	return mimeData;
}

bool AppManageModel::dropMimeData(const QMimeData *data, Qt::DropAction action, 
								  int /*row*/, int /*column*/, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction)
		return true;

	if (!data->hasFormat(kMimeType))
		return false;

	if (!parent.isValid())
		return false;
	
	int destRow = parent.row();
	int destCol = parent.column();
	QStandardItem *destItem = this->item(destRow, destCol);
	if (!destItem)
		return false;

	if (destItem->data(kPathRole).toString().isEmpty())
		return false;

	int sourceRow = 0;
	int sourceCol = 0;
	if (!mimeDataToIndex(data->data(kMimeType), sourceRow, sourceCol))
		return false;

	if (sourceRow == destRow && sourceCol == destCol)
		return true;
	
	QStandardItem *sourceItem = this->item(sourceRow, sourceCol);
	int actualData = destItem->data(kActualDataRole).toInt();
	if (actualData == AppManageModel::Data)
	{
		// switch this two item
		takeItem(sourceRow, sourceCol);
		takeItem(destRow, destCol);

		setItem(sourceRow, sourceCol, destItem);
		setItem(destRow, destCol, sourceItem);
	}
	else if (actualData == AppManageModel::Del)
	{
		// delete source item
		takeItem(sourceRow, sourceCol);
		delete sourceItem;
		sourceItem = 0;

		// move items
		int row = 0;
		int col = 0;
		int startIndex = sourceRow*kColCount + sourceCol + 1;
		int i = startIndex;
		bool hasAddItem = false;
		for (; i < kRowCount*kColCount-1; ++i)
		{
			row = i/kColCount;
			col = i%kColCount;
			QStandardItem *moveItem = item(row, col);
			if (!moveItem)
				break;

			if (moveItem->data(kPathRole).toString().isEmpty())
				break;

			if (moveItem->data(kActualDataRole).toInt() == AppManageModel::Add)
				hasAddItem = true;

			// move previous index
			takeItem(row, col);

			row = (i-1)/kColCount;
			col = (i-1)%kColCount;
			setItem(row, col, moveItem);
		}

		// check if need to add 'add' sign
		if (!hasAddItem)
		{
			setAddItem(i-1);
		}
	}

	emit appItemsChanged();

	return true;
}