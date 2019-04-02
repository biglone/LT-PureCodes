#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>
#include "widgetkit_global.h"

class WIDGETKIT_EXPORT ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget *parent = 0);
    void setText(const QString &text);
    
signals:
    
public slots:

protected:
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);

private:
	QString wrapText(const QFont &font, const QString &text, int maxWidth);

private:
    QString m_content;
    
};

#endif // ELIDEDLABEL_H
