#ifndef FRAMELESSDIALOG_H
#define FRAMELESSDIALOG_H

#include <QDialog>
#include <QPaintEvent>

class NcFramelessHelper;

class FramelessDialog : public QDialog
{
    Q_OBJECT

public:
    struct BGSizes{
        int borderwidth;
        int topRadiusX;
        int topRadiusY;
        int bottomRadiusX;
        int bottomRadiusY;
        int topBarHeight;
        int bottomBarHeight;
        int leftBarWidth;
        int rightBarWidth;
    };

    explicit FramelessDialog(QWidget *parent = 0);
    ~FramelessDialog();

    void setResizeable(const bool bAble = true);
	bool isResizeable() const;
    void setMaximizeable(const bool bAble = true);
	bool isMaximizeable() const {return m_bMaximizeable;}
    void setMoveAble(const bool bAble = true);
    void setBG(const QPixmap& bg, const BGSizes& bgSizes);
    bool isShowMaximized() const {return m_bShowMaximized;}
	int borderWidth() const;
	void setOffsetMargins(const QMargins& offsetMargins);
	bool isSupportTranslusent() const;
	void setSupportTranslusent(bool translusent);
    
public slots:
    virtual void triggerMaximize();
	virtual void setSkin() = 0;

protected slots:
	virtual void onMaximizeStateChanged(bool maximized);

signals:
    void maximizeStateChanged(const bool bMaximized);

protected:
    void paintEvent(QPaintEvent *ev);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void setMainLayout(QLayout* layout);
    void showEvent(QShowEvent *e);
	void resizeEvent(QResizeEvent *e);
    struct BGPixmaps{
        QPixmap leftBorder;
        QPixmap rightBorder;
        QPixmap topLeftCorner;
        QPixmap topLeftCornerMaximize;
        QPixmap topRightCorner;
        QPixmap topRightCornerMaximize;
        QPixmap topRadiusBorder;
        QPixmap topRadiusBorderMaximize;
        QPixmap bottomLeftCorner;
        QPixmap bottomLeftCornerMaximize;
        QPixmap bottomRightCorner;
        QPixmap bottomRightCornerMaximize;
        QPixmap bottomRadiusBorder;
        QPixmap bottomRadiusBorderMaximize;
        QPixmap topBar;
        QPixmap bottomBar;
        QPixmap leftBar;
        QPixmap rightBar;
        QPixmap central;
    };

private:
	void drawDoubleBuffer();

protected:
    NcFramelessHelper *m_fh;
    QRect m_preMaxRect;
    QPixmap m_pixmapBG;
    BGPixmaps m_bgPixmaps;
    BGSizes m_bgSizes;
    bool m_bShowMaximized;
    bool m_bMaximizeable;
    QLayout *m_mainLayout;
    QMargins m_originalMargins;
	QMargins m_offsetMargins;
	QPixmap *m_doubleBuffer;
	bool     m_supportTranslusent;
};

#endif // FRAMELESSWIDGET_H
