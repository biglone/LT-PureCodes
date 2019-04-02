#ifndef PLAINLINEEDIT_H
#define PLAINLINEEDIT_H

#include <QLineEdit>
#include "widgetkit_global.h"
#include "widgetborder.h"
#include <QColor>

class WIDGETKIT_EXPORT PlainLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PlainLineEdit(QWidget *parent = 0);
    void setText(const QString &text);
	QString wholeText() const;
	void setTextColor(const QColor &editColor, const QColor &dispColor);
	void setBorderColor(const QColor &clr);
    
public slots:
    void textEdited(const QString &txt);

protected:
    void paintEvent(QPaintEvent *ev);
	void mouseMoveEvent(QMouseEvent *ev);
	void mousePressEvent(QMouseEvent *event);

private:
	QString wrapText(const QFont &font, const QString &text, int maxWidth);

private:
	QString m_wholeText;
	QColor  m_borderColor;
	QColor  m_editColor;
	QColor  m_displayColor;
};

#endif // PLAINLINEEDIT_H
