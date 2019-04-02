#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QToolButton>
#include "widgetkit_global.h"

class WIDGETKIT_EXPORT ColorButton : public QToolButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget *parent = 0);

    const QColor & color() const;
    bool setColor(const QColor &color);
    
signals:
    void changed(const QColor& color);
    
public slots:
    void onClick();

private:
    QColor m_currentColor;
    
};

#endif // COLORBUTTON_H
