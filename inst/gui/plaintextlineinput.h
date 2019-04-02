#ifndef PLAINTEXTLINEINPUT_H
#define PLAINTEXTLINEINPUT_H

#include "framelessdialog.h"
namespace Ui {class PlainTextLineInput;};

class PlainTextLineInput : public FramelessDialog
{
	Q_OBJECT

public:
	enum MaxLengthMode
	{
		ModeUnicode,
		ModeUtf8
	};

public:
	PlainTextLineInput(QWidget *parent = 0);
	~PlainTextLineInput();

	void init(const QString &title, const QString &tip, int maxLength, MaxLengthMode mode, 
		const QString &placeText = QString(), bool allowEmpty = false);
	QString getInputText() const;

public slots:
	void setSkin();

private slots:
	void inputTextChanged(const QString &text);

private:
	Ui::PlainTextLineInput *ui;
	MaxLengthMode m_maxLengthMode;
	int           m_maxLength;
	bool          m_allowEmpty;
};

#endif // PLAINTEXTLINEINPUT_H
