#ifndef PLAINTEXTINPUT_H
#define PLAINTEXTINPUT_H

#include "framelessdialog.h"
namespace Ui {class PlainTextInput;};

class PlainTextInput : public FramelessDialog
{
	Q_OBJECT

public:
	PlainTextInput(QWidget *parent = 0);
	~PlainTextInput();

	void init(const QString &title, const QString &tip, int maxLength, const QString &placeText = QString());
	QString getInputText() const;

public slots:
	void setSkin();

private slots:
	void inputTextChanged();

private:
	Ui::PlainTextInput *ui;
	int m_maxLength;
};

#endif // PLAINTEXTINPUT_H
