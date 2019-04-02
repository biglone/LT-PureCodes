#ifndef SELECTCHATDIALOG_H
#define SELECTCHATDIALOG_H

#include "framelessdialog.h"
#include "bean/bean.h"
namespace Ui {class SelectChatDialog;};

class SelectChatDialog : public FramelessDialog
{
	Q_OBJECT

public:
	SelectChatDialog(const QString &title, QWidget *parent = 0);
	~SelectChatDialog();

	void getSelect(bean::MessageType &selType, QString &selId);

public Q_SLOTS:
	virtual void setSkin();

private Q_SLOTS:
	void onChatSelected(const QString &id, int source);

private:
	void initUI();

private:
	Ui::SelectChatDialog *ui;

	QString               m_selectId;
	bean::MessageType     m_selectType;
};

#endif // SELECTCHATDIALOG_H
