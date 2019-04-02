#ifndef BLACKLISTDIALOG_H
#define BLACKLISTDIALOG_H

#include "FramelessDialog.h"
#include <QPointer>
namespace Ui {class BlackListDialog;};

class BlackListDialog : public FramelessDialog
{
	Q_OBJECT

public:
	BlackListDialog(QWidget *parent = 0);
	~BlackListDialog();

	static BlackListDialog *getBlackListDialog();

Q_SIGNALS:
	void removeBlack(const QString &id);
	void viewMaterial(const QString &id);

public slots:
	virtual void setSkin();

private slots:
	void onRemoveBlack(const QString &id);

private:
	Ui::BlackListDialog *ui;

	static QPointer<BlackListDialog> s_dlg;
};

#endif // BLACKLISTDIALOG_H
