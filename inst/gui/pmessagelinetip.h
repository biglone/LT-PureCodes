#ifndef PMESSAGELINETIP_H
#define PMESSAGELINETIP_H

#include "framelessdialog.h"
#include <QString>
#include <QTimer>
#include <QFont>

class QFocusEvent;
class QPaintEvent;

class PMessageLineTip : public FramelessDialog
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

public:
	PMessageLineTip(Type tipType, const QString &text, QWidget *parent = 0);
	~PMessageLineTip();

	void setTipTimeout(int ms = 8000);
	QSize sizeHint() const;

public slots:
	void setSkin();

protected:
	void focusOutEvent(QFocusEvent *ev);
	void paintEvent(QPaintEvent *ev);

private slots:
	void onHideTimeout();

private:
	QTimer                m_hideTimer;
	PMessageLineTip::Type m_tipType;
	QString               m_text;
	QFont                 m_textFont;
};

#endif // PMESSAGELINETIP_H
