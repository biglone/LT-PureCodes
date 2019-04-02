#ifndef AVATARLABEL_H
#define AVATARLABEL_H

#include <QLabel>

class AvatarLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AvatarLabel(QWidget *parent = 0);
	void setClickable(bool bAble = true);
	void setHoverPixmap(const QPixmap &hoverPixmap);
    
signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
	void paintEvent(QPaintEvent *ev);

private:
    bool m_bPreesed;
	bool m_bClickable;
	QPixmap m_hoverPixmap;
};

#endif // HOVERLABEL_H
