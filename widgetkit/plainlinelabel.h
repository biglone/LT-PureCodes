#ifndef PLAINLINELABEL_H
#define PLAINLINELABEL_H

#include <QLabel>
#include "widgetkit_global.h"
#include "widgetborder.h"

class WIDGETKIT_EXPORT PlainLineLabel : public QLabel
{
	Q_OBJECT

public:
	PlainLineLabel(QWidget *parent);
	~PlainLineLabel();

	void setBorderStyle(const WidgetBorder &borderHover, const QPixmap &pixmapHover);

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);
	void enterEvent(QEvent *ev);
	void leaveEvent(QEvent *ev);
	void paintEvent(QPaintEvent *ev);

private:
	QString wrapText(const QFont &font, const QString &text, int maxWidth);

private:
	bool m_bPreesed;
	bool m_bHovered;
	QPixmap m_pixmapHover;
	WidgetBorder m_borderHover;
};

#endif // PLAINLINELABEL_H
