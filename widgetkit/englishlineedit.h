#ifndef ENGLISHLINEEDIT_H
#define ENGLISHLINEEDIT_H

#include <QLineEdit>
#include "widgetkit_global.h"

/*
#ifdef Q_OS_WIN
#include <imm.h>
#endif // Q_OS_WIN
*/

class WIDGETKIT_EXPORT EnglishLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	EnglishLineEdit(QWidget *parent = 0);
	~EnglishLineEdit();

protected:
	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);

#ifdef Q_OS_WIN
private:
	unsigned int m_hImcId;
#endif // Q_OS_WIN
};

#endif // ENGLISHLINEEDIT_H
