#ifndef SHORTCUTCONFLICTDLG_H
#define SHORTCUTCONFLICTDLG_H

#include "framelessdialog.h"
#include <QPointer>
namespace Ui {class ShortcutConflictDlg;};

class ShortcutConflictDlg : public FramelessDialog
{
	Q_OBJECT

public:
	static ShortcutConflictDlg* instance(int failedCount);
	ShortcutConflictDlg(QWidget *parent = 0);
	~ShortcutConflictDlg();

	void setFailedCount(int failedCount);

signals:
	void modifyShortcutKey();

public slots:
	void setSkin();

private slots:
	void on_checkBoxNotRemind_toggled(bool checked);
	void on_labelModify_clicked();

private:
	Ui::ShortcutConflictDlg *ui;

	static QPointer<ShortcutConflictDlg> s_instance;
};

#endif // SHORTCUTCONFLICTDLG_H
