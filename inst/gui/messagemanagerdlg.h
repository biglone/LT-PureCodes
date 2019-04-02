#ifndef MESSAGEMANAGERDLG_H
#define MESSAGEMANAGERDLG_H

#include <QModelIndex>
#include "framelessdialog.h"
#include <QDate>
#include "bean/bean.h"

namespace Ui {class MessageManagerDlg;};

class MsgManagerListModel;
class MessageManagerDlg : public FramelessDialog
{
	Q_OBJECT

public:
	static MessageManagerDlg *instance();
	virtual ~MessageManagerDlg();

	void init(const QString &id = "", int msgType = bean::Message_Chat, const QString &extra = "");

public slots:
	void setSkin();

private slots:
	void on_btnSearch_clicked();
	void on_lineEditWord_returnPressed();
	void onMaximizeStateChanged(bool isMaximized);
	void onComboBoxDateActivated(int index);
	void onMessageIdTypeChanged(int msgType, const QString &id);
	void chatSelected(const QString &id, int source);

private:
	explicit MessageManagerDlg(QWidget *parent = 0);
	void initUI();
	void initSignals();

private:
	Ui::MessageManagerDlg    *ui;
	int                       m_sourceType;
	QString                   m_sourceId;
	QDate                     m_dateFrom;
	QDate                     m_dateTo;

	static MessageManagerDlg *s_instance;
};

#endif // MESSAGEMANAGERDLG_H
