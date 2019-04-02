#include "shortcutlineedit.h"
#include <QtGui/QKeyEvent>
#ifdef Q_OS_WIN
#include <Windows.h>
#include <imm.h>
#endif // Q_OS_WIN

ShortcutLineEdit::ShortcutLineEdit(QWidget *parent) :
    FilterLineEdit(parent),
    m_keyNum(0)
{
    m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;

    connect(this, SIGNAL(buttonClicked(FancyLineEdit::Side)), SLOT(clearKeySequence()));
}

QKeySequence ShortcutLineEdit::keySequence() const
{
    return QKeySequence(text());
}

void ShortcutLineEdit::setKeySequence(const QKeySequence &key)
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    m_keyNum = key.count();
    for (int i = 0; i < m_keyNum; ++i) {
        m_key[i] = key[i];
    }

	setText(key.toString());
}

void ShortcutLineEdit::clearKeySequence()
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    m_keyNum = 0;
    FilterLineEdit::clear();
}

bool ShortcutLineEdit::event(QEvent *e)
{
    if ( e->type() == QEvent::Shortcut || e->type() == QEvent::KeyRelease ) {
        return true;
    }

    if (e->type() == QEvent::ShortcutOverride) {
        // for shortcut overrides, we need to accept as well
        e->accept();
        return true;
    }

    return QLineEdit::event(e);
}

void ShortcutLineEdit::keyPressEvent(QKeyEvent *e)
{
    int nextKey = e->key();
    if ( m_keyNum > 3 ||
         nextKey == Qt::Key_Control ||
         nextKey == Qt::Key_Shift ||
         nextKey == Qt::Key_Meta ||
         nextKey == Qt::Key_Alt )
         return;

	// backspace, escape, delete and space key clear the sequence
	if (nextKey == Qt::Key_Backspace ||
		nextKey == Qt::Key_Escape ||
		nextKey == Qt::Key_Space ||
		nextKey == Qt::Key_Tab ||
		nextKey == Qt::Key_Backtab ||
		nextKey == Qt::Key_Delete ||
		nextKey == Qt::Key_Enter ||
		nextKey == Qt::Key_Return)
	{
		clearKeySequence();
		return;
	}

	// only allow on key sequence
	if ( m_keyNum >= 1 )
	{
		clearKeySequence();
	}

	QString keyText = e->text();
	if (!hasModifiers(e->modifiers(), keyText))
	{
		if (!(nextKey >= Qt::Key_F1 && nextKey <= Qt::Key_F35)) 
		{
			// not F1 - F12, add default modifiers
			nextKey |= Qt::CTRL;
			nextKey |= Qt::SHIFT;
		}
	}
	else
	{
		nextKey |= translateModifiers(e->modifiers(), keyText);
	}

    switch (m_keyNum) {
        case 0:
            m_key[0] = nextKey;
            break;
        case 1:
            m_key[1] = nextKey;
            break;
        case 2:
            m_key[2] = nextKey;
            break;
        case 3:
            m_key[3] = nextKey;
            break;
        default:
            break;
    }
    m_keyNum++;
    QKeySequence ks(m_key[0], m_key[1], m_key[2], m_key[3]);
    setText(ks.toString());
    e->accept();
}

void ShortcutLineEdit::focusInEvent(QFocusEvent *event)
{
	QLineEdit::focusInEvent(event);

#ifdef Q_OS_WIN
	QWidget *pWidget = this->window();
	HWND hWnd = (HWND)(pWidget->winId());
	m_hImcId = (unsigned int)ImmAssociateContext(hWnd, NULL);
#endif // Q_OS_WIN
}

void ShortcutLineEdit::focusOutEvent(QFocusEvent *event)
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

int ShortcutLineEdit::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier) && (text.size() == 0
                                        || !text.at(0).isPrint()
                                        || text.at(0).isLetterOrNumber()
                                        || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

bool ShortcutLineEdit::hasModifiers(Qt::KeyboardModifiers state, const QString &text)
{
	// The shift modifier only counts when it is not used to type a symbol
	// that is only reachable using the shift key anyway
	if ((state & Qt::ShiftModifier) && (text.size() == 0
		|| !text.at(0).isPrint()
		|| text.at(0).isLetterOrNumber()
		|| text.at(0).isSpace()))
		return true;
	if (state & Qt::ControlModifier)
		return true;
	if (state & Qt::MetaModifier)
		return true;
	if (state & Qt::AltModifier)
		return true;
	return false;
}
