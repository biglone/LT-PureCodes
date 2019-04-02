#ifndef STATUSLABEL_H
#define STATUSLABEL_H

#include <QLabel>

class StatusLabel : public QLabel
{
    Q_OBJECT
public:
    explicit StatusLabel(QWidget *parent = 0);
	void setClickable(bool bAble = true);
    
signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
	void paintEvent(QPaintEvent *ev);

private:
    bool m_bPreesed;
	bool m_bClickable;
};

#endif // STATUSLABEL_H
