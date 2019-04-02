#ifndef STYLETESTDIALOG_H
#define STYLETESTDIALOG_H

#include <QDialog>
namespace Ui {class StyleTestDialog;};

class StyleTestDialog : public QDialog
{
	Q_OBJECT

public:
	static StyleTestDialog *instance();
	~StyleTestDialog();

private:
	void initUI();
	void initStyle();
	StyleTestDialog(QWidget *parent = 0);

private slots:
	void on_toolButtonMenu_clicked();

private:
	Ui::StyleTestDialog *ui;
};

#endif // STYLETESTDIALOG_H
