#ifndef FILTEROKLINEEDIT_H
#define FILTEROKLINEEDIT_H

#include "widgetkit_global.h"

#include "fancylineedit.h"

class WIDGETKIT_EXPORT FilterOkLineEdit : public FancyLineEdit
{
	Q_OBJECT

public:
	explicit FilterOkLineEdit(QWidget *parent = 0);
	void setClearRightButton();
	void setOkRightButton();

signals:
	void filterChanged(const QString &);

private slots:
	void slotTextChanged();

private:
	QString m_lastFilterText;
};

#endif // FILTEROKLINEEDIT_H
