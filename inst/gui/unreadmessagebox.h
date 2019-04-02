#ifndef UNREADMESSAGEBOX_H
#define UNREADMESSAGEBOX_H

#include "framelessdialog.h"
#include "ui_unreadmessagebox.h"
#include <QSortFilterProxyModel>

class CSystemTray;
class UnreadMsgModel;
class UnreadMessageSortFilterModel;

class UnreadMessageBox : public FramelessDialog
{
	Q_OBJECT

public:
	UnreadMessageBox(UnreadMsgModel *model, CSystemTray *trayIcon, QWidget *parent = 0);
	~UnreadMessageBox();

public slots:
	void onTrayIconHoverEnter(const QRect &trayIconGeometry);
	void onTrayIconHoverLeave();
	void onUnreadItemCountChanged();
	void ignore();
	void viewAll();
	void onItemClicked(const QModelIndex &index);
	void ignoreItem(const QModelIndex &sourceIndex);

	virtual void setSkin();

signals:
	void openChat(const QString &id);
	void openGroupChat(const QString &id);
	void openDiscuss(const QString &id);
	void openAllUnreadChats(const QList<int> &msgTypes, const QStringList &ids);

protected:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

private slots:
	void onPreTakeMsg(const QModelIndex &sourceIndex);
	
private:
	void readdItemWidget();

private:
	Ui::UnreadMessageBox         ui;
	UnreadMsgModel               *m_unreadMessageModel;
	UnreadMessageSortFilterModel *m_unreadMessageFilterModel;
	CSystemTray                  *m_trayIcon;
	QTimer                       *m_preHideTimer;
};

#endif // UNREADMESSAGEBOX_H
