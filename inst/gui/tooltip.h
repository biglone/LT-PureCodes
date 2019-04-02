#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QLabel>

class CToolTip : public QLabel
{
	Q_OBJECT
public:
	explicit CToolTip(QWidget *parent = 0);
	~CToolTip();

public Q_SLOTS:
	void setText(const QString &);

//public:
//	void setText(const QString&);
//	QString text();

//private:
//	Ui::CToolTip *ui;
};

#endif // TOOLTIP_H
