#ifndef _VIDEOWIDGET_H_
#define _VIDEOWIDGET_H_

#include <QWidget>

class VideoWidgetPrivate;
class VideoWidget : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(VideoWidget)

public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();

    QSize imageSize() const;

    virtual QPaintEngine* paintEngine() const;

signals:
	void videoDoubleClicked();

public slots:
    void onUpdated(const QImage &frame);
    void onSizeChanged(const QSize &s);

protected:
    void paintEvent(QPaintEvent *e);
    void showEvent(QShowEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);

    bool event(QEvent *e);

private:
    QScopedPointer<VideoWidgetPrivate> d_ptr;
};
#endif //_VIDEOWIDGET_H_
