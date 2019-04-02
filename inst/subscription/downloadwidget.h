#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>
#include "ui_downloadwidget.h"
#include <QStandardItemModel>

class DownloadItemWidget;
class QNetworkRequest;
class QNetworkReply;
class QNetworkAccessManager;

class DownloadWidget : public QWidget
{
	Q_OBJECT

public:
	DownloadWidget(QWidget *parent = 0);
	~DownloadWidget();

	void addItem(const QString &urlStr, const QString &name = QString());
	static QString saveFileName(QNetworkReply *reply);

public slots:
	void removeItem(DownloadItemWidget *itemWidget);
	void removeItems();
	void selectAllItems();
	void deselectAllItems();
	void removeAllItems();

private slots:
	void changeSaveDir();

private:
	DownloadItemWidget* addItem(QNetworkReply *reply, const QString &name = QString());

private:
	Ui::DownloadWidget                        ui;
	QStandardItemModel                       *m_model;
	QMap<DownloadItemWidget*, QStandardItem*> m_itemsMap;
	QMap<DownloadItemWidget*, QString>        m_itemUrls;
	QNetworkAccessManager                    *m_nam;
};


#include <QItemDelegate>

class DownloadItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit DownloadItemDelegate(QObject *parent = 0);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DOWNLOADWIDGET_H
