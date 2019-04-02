#ifndef PMESSAGEBOX_H
#define PMESSAGEBOX_H

#include "framelessdialog.h"
#include <QDialogButtonBox>

namespace Ui {class PMessageBox;};

class PMessageBox : public FramelessDialog
{
	Q_OBJECT

public:
	enum Type{
		Success,
		Failed,
		Question,
		Information,
		Warning
	};

	static QDialogButtonBox::StandardButton critical(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
	static QDialogButtonBox::StandardButton information(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
	static QDialogButtonBox::StandardButton question(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
	static QDialogButtonBox::StandardButton warning(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
	static QDialogButtonBox::StandardButton success(QWidget *parent, const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);

public:
	PMessageBox(Type type, const QString &tip, QDialogButtonBox::StandardButtons btns, const QString &title, QWidget *parent = 0);
	~PMessageBox();

	QDialogButtonBox::ButtonRole clickedButtonRole() {return m_clickedRole;}
	QDialogButtonBox::StandardButton clickedButton() {return m_clickedButton;}

public slots:
	void setSkin();

protected slots:
	void buttonClicked(QAbstractButton * button);

private:
	Ui::PMessageBox *ui;
	QDialogButtonBox::ButtonRole     m_clickedRole;
	QDialogButtonBox::StandardButton m_clickedButton;
};

#endif // PMESSAGEBOX_H
