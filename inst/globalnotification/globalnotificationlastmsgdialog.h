#ifndef GLOBALNOTIFICATIONLASTMSGDIALOG_H
#define GLOBALNOTIFICATIONLASTMSGDIALOG_H

#include "framelessdialog.h"
#include <QPointer>

namespace Ui {class GlobalNotificationLastMsgDialog;};
class GlobalNotificationLastMsgModel;
class QAction;

class GlobalNotificationLastMsgDialog : public FramelessDialog
{
	Q_OBJECT

public:
	GlobalNotificationLastMsgDialog(QWidget *parent = 0);
	~GlobalNotificationLastMsgDialog();

	static GlobalNotificationLastMsgDialog *getDialog();
	static bool hasDialog();

Q_SIGNALS:
	void openGlobalNotificationMsg(const QString &id);
	void openGlobalNotificationDetail(const QString &id);

public slots:
	virtual void setSkin();

private slots:
	void onItemDoubleClicked(const QModelIndex &index);
	void onGlobalNotificationLogoChanged(const QString &globalNotificationId);
	void onMsgAction();
	void onDetailAction();
	void onDeleteAction();
	void contextMenu(const QPoint &position);
	void onGlobalNotificationLastMsgChanged();

private:
	void initUI();

private:
	Ui::GlobalNotificationLastMsgDialog                *ui;
	QPointer<GlobalNotificationLastMsgModel>            m_listModel;
	static QPointer<GlobalNotificationLastMsgDialog>    s_dialog;

	QAction *m_msgAction;
	QAction *m_detailAction;
	QAction *m_deleteAction;
};

#endif // GLOBALNOTIFICATIONLASTMSGDIALOG_H
