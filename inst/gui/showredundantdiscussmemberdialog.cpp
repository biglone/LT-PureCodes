#include <QAbstractItemModel>
#include <QSTringListModel>
#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QPainter>
#include <QBitmap>
#include <QDebug>

#include "model/ModelManager.h"
#include "PmApp.h"


#include "showredundantdiscussmemberdialog.h"

SimpleDiscussItemDelegate::SimpleDiscussItemDelegate(QObject *parent)
	: QItemDelegate(parent)
{

}

SimpleDiscussItemDelegate::~SimpleDiscussItemDelegate()
{

}

QSize SimpleDiscussItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);
	return QSize(33, 33);
}

void SimpleDiscussItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QColor textColor(Qt::black);
	QColor deptColor(128, 128, 128);
	QColor bgColor;

	QRect paintRect = option.rect;

	if (option.state & QStyle::State_Selected)
	{
		bgColor = QColor("#fff5cc");
	}
	else if (option.state & QStyle::State_MouseOver)
	{
		bgColor = QColor("#dfeefa");
	}
	else 
	{
		bgColor = QColor(247, 247, 247);
	}

	painter->fillRect(paintRect, bgColor);

	QPixmap avatar =  QPixmap(":/images/Icon_64_small.png");
	QSize avatarSize(20, 20);

	paintRect = option.rect;

	paintRect.setTop(paintRect.top() + (paintRect.height() - avatar.height())/2);
	paintRect.setLeft(paintRect.left() + 10);
	paintRect.setHeight(avatar.height());
	paintRect.setWidth(avatar.width());
	QBitmap avatarMask(":/images/Icon_60_mask20.png");
	avatar.setMask(avatarMask);
	painter->drawPixmap(paintRect, avatar);

	painter->setPen(textColor);
	QString text = index.data(Qt::DisplayRole).toString();
	paintRect.setLeft(paintRect.left() + avatar.width() + 15);
	paintRect.setWidth(160);

	QFontMetrics fontMetrics = painter->fontMetrics();
	QString txt = fontMetrics.elidedText(text, Qt::ElideRight, 140);
	painter->drawText(paintRect, Qt::AlignLeft|Qt::AlignVCenter, txt);
}

ShowRedundantDiscussMemberDialog::ShowRedundantDiscussMemberDialog(QStringList &redundantIdList, QWidget *parent)
	: FramelessDialog(parent)
{
	ui.setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowIcon(qApp->windowIcon());
	setWindowTitle(tr("Whether to create new discuss with identical members?"));
	setMainLayout(ui.verticalLayoutMain);
	setFixedSize(260, 350);
	setResizeable(false);
	setSkin();
	ui.labelTitle->setWordWrap(true);

	ModelManager *modelManager = qPmApp->getModelManager();
	QStringList names;
	foreach(QString id, redundantIdList)
	{
		names << modelManager->discussName(id);
	}

	QAbstractItemModel *model = new QStringListModel(names);
	SimpleDiscussItemDelegate *delegate = new SimpleDiscussItemDelegate(this);
	ui.listView->setModel(model);
	ui.listView->setItemDelegate(delegate);
	ui.listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

ShowRedundantDiscussMemberDialog::~ShowRedundantDiscussMemberDialog()
{

}

void ShowRedundantDiscussMemberDialog::setSkin()
{
	QPixmap bgPixmap(":/theme/dialog_bgs/dialog_bg_9.png");
	FramelessDialog::BGSizes bgSizes = {0};
	bgSizes.borderwidth = 5;
	bgSizes.topBarHeight = 33;
	bgSizes.bottomBarHeight = 33;
	setBG(bgPixmap, bgSizes);

	ui.labelTitle->setStyleSheet("QLabel {font-size: 12pt; color: white;}");
	setStyleSheet("QWidget#topBar{background: rgb(1, 120, 216);}");
}

void ShowRedundantDiscussMemberDialog::on_btnOK_clicked()
{
	accept();
}
void ShowRedundantDiscussMemberDialog::on_btnCancel_clicked()
{
	close();
}
