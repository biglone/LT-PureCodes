#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include "widgetkit_global.h"

class WIDGETKIT_EXPORT ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = 0);
    void setFontAtt(const QColor &normalClr, int fontSize, bool bold);
	void setEnterUnderline(bool on);
	void setClickable(bool able);
    
signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
	void enterEvent(QEvent *ev);
	void leaveEvent(QEvent *ev);

private:
    bool    m_bPreesed;
	QColor  m_normalClr;
	int     m_fontSize;
	bool    m_bold;
	bool    m_enterUnderline;
	bool    m_clickable;
};

#endif // CLICKABLELABEL_H
