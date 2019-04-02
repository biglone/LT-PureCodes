#ifndef SECRETSWITCH_H
#define SECRETSWITCH_H

#include <QWidget>
#include <QPixmap>
#include <QRect>
#include <QTimer>

class SecretSwitch : public QWidget
{
	Q_OBJECT

	enum PressAction
	{
		NoneAction,
		Press,
		Drag
	};

public:
	SecretSwitch(QWidget *parent = 0);
	~SecretSwitch();

	void setChecked(bool checked);
	bool isChecked() const;

Q_SIGNALS:
	void toggled(bool checked);

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void paintEvent(QPaintEvent *ev);

private:
	QRect switchRect() const;

private:
	bool        m_checked;
	QRect       m_leftRect;
	QRect       m_rightRect;
	QPoint      m_prevPt;
	PressAction m_pressAction;
	int         m_switchLocation;
	QPixmap     m_switchOnPixmap;
	QPixmap     m_switchOffPixmap;
	QPixmap     m_switchOnBackPixmap;
	QPixmap     m_switchOffBackPixmap;
};

class SecretSwitchTip : public QWidget
{
	Q_OBJECT

public:
	SecretSwitchTip(QWidget *parent = 0);

public slots:
	void showOnTip(const QPoint &pt);
	void showOffTip(const QPoint &pt);
	void hideTip();

protected:
	void paintEvent(QPaintEvent *ev);

private:
	QPixmap m_onTipPixmap;
	QPixmap m_offTipPixmap;
	QTimer  m_showTimer;
	bool    m_onTip;
};

#endif // SECRETSWITCH_H
