#ifndef SHORTCUTLINEEDIT_H
#define SHORTCUTLINEEDIT_H

#include "filterlineedit.h"

class WIDGETKIT_EXPORT ShortcutLineEdit : public FilterLineEdit
{
    Q_OBJECT

public:
    explicit ShortcutLineEdit(QWidget *parent = 0);

    QKeySequence keySequence() const;
    void setKeySequence(const QKeySequence &key);

public slots:
    void clearKeySequence();

protected:
    bool event(QEvent *);
    void keyPressEvent(QKeyEvent *);

	void focusInEvent(QFocusEvent *event);
	void focusOutEvent(QFocusEvent *event);

private:
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
	bool hasModifiers(Qt::KeyboardModifiers state, const QString &text);

private:
    int m_keyNum;
    int m_key[4];

#ifdef Q_OS_WIN
	unsigned int m_hImcId;
#endif // Q_OS_WIN

};

#endif // SHORTCUTLINEEDIT_H
