#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H

#include "widgetkit_global.h"
#include "fancylineedit.h"

class KeyDelegate;

class WIDGETKIT_EXPORT FilterLineEdit : public FancyLineEdit
{
    Q_OBJECT

public:
    explicit FilterLineEdit(QWidget *parent = 0);

	void setKeyDelegate(KeyDelegate *keyDelegate);

protected:
	void keyPressEvent(QKeyEvent *e);

signals:
    void filterChanged(const QString &);
	void filterChanged(const QString &newText, const QString &oldText);

private slots:
    void slotTextChanged();

private:
    QString m_lastFilterText;
	KeyDelegate *m_keyDelegate;
};

#endif // FILTERLINEEDIT_H
