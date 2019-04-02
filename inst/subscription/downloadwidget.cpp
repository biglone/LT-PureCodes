#include "downloadwidget.h"
#include "downloaditemwidget.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QFileInfo>
#include "Account.h"
#include "settings/AccountSettings.h"
#include "util/FileDialog.h"
#include <QFileDialog>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS FROM CLASS DownloadWidget
DownloadWidget::DownloadWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.listViewDownloadItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.listViewDownloadItems->setDragEnabled(false);
	ui.listViewDownloadItems->setItemDelegate(new DownloadItemDelegate(ui.listViewDownloadItems));
	m_model = new QStandardItemModel(this);
	ui.listViewDownloadItems->setModel(m_model);

	this->setStyleSheet(
		"QListView#listViewDownloadItems {"
		"	background: transparent;"
		"	border: none;"
		"}"
		"QWidget#dirPanel {"
		"	margin-left: 20px;"
		"	margin-right: 20px;"
		"	border: none;"
		"	border-bottom: 1px solid rgb(219, 219, 219);"
		"}"
		/*
		"QWidget#removePanel {"
		"	margin-left: 20px;"
		"	margin-right: 20px;"
		"	border: none;"
		"	border-top: 1px solid rgb(219, 219, 219);"
		"}"
		*/
		);

	m_nam = new QNetworkAccessManager(this);

	QString saveDir = Account::settings()->subscriptionAttachDir();
	ui.labelDir->setText(saveDir);
	connect(ui.pushButtonDir, SIGNAL(clicked()), this, SLOT(changeSaveDir()));

	ui.labelSelectAll->setFontAtt(QColor(0, 120, 216), 10, false);
	connect(ui.labelSelectAll, SIGNAL(clicked()), this, SLOT(selectAllItems()));

	ui.labelDeselectAll->setFontAtt(QColor(0, 120, 216), 10, false);
	connect(ui.labelDeselectAll, SIGNAL(clicked()), this, SLOT(deselectAllItems()));

	ui.labelRemove->setFontAtt(QColor(0, 120, 216), 10, false);
	connect(ui.labelRemove, SIGNAL(clicked()), this, SLOT(removeItems()));

	ui.labelRemoveAll->setFontAtt(QColor(0, 120, 216), 10, false);
	connect(ui.labelRemoveAll, SIGNAL(clicked()), this, SLOT(removeAllItems()));
}

DownloadWidget::~DownloadWidget()
{

}

void DownloadWidget::addItem(const QString &urlStr, const QString &name /*= QString()*/)
{
	if (urlStr.isEmpty())
		return;

	// check if download before
	DownloadItemWidget *origWidget = 0;
	foreach (DownloadItemWidget *widget, m_itemUrls.keys())
	{
		QString urlString = m_itemUrls[widget];
		if (urlString == urlStr)
		{
			origWidget = widget;
			break;
		}
	}

	// download before need flash this widget
	if (origWidget)
	{
		QStandardItem *item = m_itemsMap[origWidget];
		ui.listViewDownloadItems->scrollTo(item->index());
		origWidget->flash();
		return;
	}

	// do download
	QNetworkRequest request(QUrl::fromUserInput(urlStr));
	DownloadItemWidget *itemWidget = addItem(m_nam->get(request), name);
	if (itemWidget)
	{
		m_itemUrls.insert(itemWidget, urlStr);
	}
}

void DownloadWidget::removeItem(DownloadItemWidget *itemWidget)
{
	if (itemWidget && m_itemsMap.contains(itemWidget))
	{
		m_model->removeRow(m_itemsMap[itemWidget]->row());
		m_itemsMap.remove(itemWidget);
		m_itemUrls.remove(itemWidget);
		itemWidget->deleteLater();
	}
}

void DownloadWidget::removeItems()
{
	QList<DownloadItemWidget *> removeWidges;
	int i = 0;
	for (i = 0; i < m_model->rowCount(); ++i)
	{
		QStandardItem *item = m_model->item(i);
		DownloadItemWidget *widget = qobject_cast<DownloadItemWidget *>(ui.listViewDownloadItems->indexWidget(item->index()));
		if (widget && widget->isChecked())
		{
			removeWidges.insert(0, widget);
		}
	}

	for (i = 0; i < removeWidges.count(); ++i)
	{
		DownloadItemWidget *widget = removeWidges[i];
		removeItem(widget);
	}
}

void DownloadWidget::selectAllItems()
{
	for (int i = 0; i < m_model->rowCount(); ++i)
	{
		QStandardItem *item = m_model->item(i);
		DownloadItemWidget *widget = qobject_cast<DownloadItemWidget *>(ui.listViewDownloadItems->indexWidget(item->index()));
		if (widget)
		{
			widget->setChecked(true);
		}
	}
}

void DownloadWidget::deselectAllItems()
{
	for (int i = 0; i < m_model->rowCount(); ++i)
	{
		QStandardItem *item = m_model->item(i);
		DownloadItemWidget *widget = qobject_cast<DownloadItemWidget *>(ui.listViewDownloadItems->indexWidget(item->index()));
		if (widget)
		{
			widget->setChecked(false);
		}
	}
}

void DownloadWidget::removeAllItems()
{
	QList<DownloadItemWidget *> removeWidges;
	int i = 0;
	for (i = 0; i < m_model->rowCount(); ++i)
	{
		QStandardItem *item = m_model->item(i);
		DownloadItemWidget *widget = qobject_cast<DownloadItemWidget *>(ui.listViewDownloadItems->indexWidget(item->index()));
		if (widget)
		{
			removeWidges.insert(0, widget);
		}
	}

	for (i = 0; i < removeWidges.count(); ++i)
	{
		DownloadItemWidget *widget = removeWidges[i];
		removeItem(widget);
	}
}

void DownloadWidget::changeSaveDir()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Dir"), ui.labelDir->text());
	if (!dir.isEmpty())
	{
		dir = QDir::toNativeSeparators(dir);
		ui.labelDir->setText(dir);
		
		Account::settings()->setSubscriptionAttachDir(dir);
	}
}

DownloadItemWidget* DownloadWidget::addItem(QNetworkReply *reply, const QString &name /*= QString()*/)
{
	if (!reply || reply->url().isEmpty())
		return 0;

	QString fileName = name;
	if (fileName.isEmpty())
		fileName = saveFileName(reply);
	fileName = ui.labelDir->text() + "\\" + fileName;
	QStandardItem* item = new QStandardItem;
	m_model->appendRow(item);
	DownloadItemWidget *itemWidget = new DownloadItemWidget(fileName, reply, m_nam, false, ui.listViewDownloadItems);
	ui.listViewDownloadItems->setIndexWidget(item->index(), itemWidget);
	m_itemsMap[itemWidget] = item;
	connect(itemWidget, SIGNAL(removed()), SLOT(itemRemoved()));
	return itemWidget;
}

QString DownloadWidget::saveFileName(QNetworkReply *reply)
{
    QString path = reply->url().path();
    QFileInfo info(path);
    QString baseName = info.completeBaseName();
    QString endName = info.suffix();
	if (baseName.isEmpty()) {
		baseName = QLatin1String("unnamed_download");
		qDebug() << "DownloadManager:: downloading unknown file:" << reply->url();
	}
	QString name = baseName + QLatin1Char('.') + endName;
	return name;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS DownloadItemDelegate
DownloadItemDelegate::DownloadItemDelegate(QObject *parent) :
QItemDelegate(parent)
{
}


QSize DownloadItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option);
	Q_UNUSED(index);

	QSize sz = QSize(208, 58);
	return sz;
}

void DownloadItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(index);

	QListView *view = static_cast<QListView *>(parent());
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

	return;
}
