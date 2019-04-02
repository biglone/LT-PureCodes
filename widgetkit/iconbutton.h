#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include "widgetkit_global.h"
#include <QAbstractButton>

class WIDGETKIT_EXPORT IconButton: public QAbstractButton
{
    Q_OBJECT
    Q_DISABLE_COPY(IconButton)

    Q_PROPERTY(bool autoHide READ hasAutoHide WRITE setAutoHide)
    Q_PROPERTY(float iconOpacity READ iconOpacity WRITE setIconOpacity)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)

public:
    explicit IconButton(QWidget *parent = 0);

    void animateShow(bool visible);

    bool hasAutoHide() const { return m_autoHide; }
    void setAutoHide(bool hide) { m_autoHide = hide; }

    float iconOpacity() { return m_iconOpacity; }
    void setIconOpacity(float value) { m_iconOpacity = value; update(); }

    QPixmap pixmap() const { return m_pixmap; }
    void setPixmap(const QPixmap &pixmap) { m_pixmap = pixmap; update(); }

protected:
    void paintEvent(QPaintEvent *event);

private:
    float m_iconOpacity;
    bool m_autoHide;
    QPixmap m_pixmap;
};

#endif // ICONBUTTON_H
