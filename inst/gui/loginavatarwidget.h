#ifndef LOGINAVATARWIDGET_H
#define LOGINAVATARWIDGET_H

#include <QWidget>
#include <QPixmap>
#include "widgetborder.h"

class LoginAvatarWidget : public QWidget
{
	Q_OBJECT

public:
	LoginAvatarWidget(QWidget *parent = 0);
	~LoginAvatarWidget();

	void setPixmap(const QPixmap &pixmap);
	void setBorder(const QPixmap &pixmapBorder, WidgetBorder border);

protected:
	void paintEvent(QPaintEvent *ev);

private:
	QPixmap      m_pixmap;
	QPixmap      m_pixmapBorder;
	WidgetBorder m_border;
};

#endif // LOGINAVATARWIDGET_H
