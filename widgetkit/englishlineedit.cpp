#include "englishlineedit.h"
#ifdef Q_OS_WIN
#include <Windows.h>
#include <imm.h>
#endif // Q_OS_WIN

EnglishLineEdit::EnglishLineEdit(QWidget *parent /*= 0*/)
	: QLineEdit(parent)
{
#ifdef Q_OS_WIN
	m_hImcId = 0;
#endif // Q_OS_WIN
}

EnglishLineEdit::~EnglishLineEdit()
{

}

void EnglishLineEdit::focusInEvent(QFocusEvent *event)
{
	QLineEdit::focusInEvent(event);

#ifdef Q_OS_WIN
	QWidget *pWidget = this->window();
	HWND hWnd = (HWND)(pWidget->winId());
	m_hImcId = (unsigned int)ImmAssociateContext(hWnd, NULL);
#endif // Q_OS_WIN
}

void EnglishLineEdit::focusOutEvent(QFocusEvent *event)
{
	QLineEdit::focusOutEvent(event);

#ifdef Q_OS_WIN	
	QWidget *pWidget = this->window();
	HWND hWnd = (HWND)(pWidget->winId());
	if (m_hImcId)
	{
		ImmAssociateContext(hWnd, (HIMC)m_hImcId); 
	}
#endif // Q_OS_WIN
}

