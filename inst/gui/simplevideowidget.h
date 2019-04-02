#ifndef SIMPLEVIDEOWIDGET_H
#define SIMPLEVIDEOWIDGET_H

#include <QWidget>
#include <QSize>
#include <QImage>

class QPixmap;
class QImage;
class QPainter;

class SimpleVideoWidget : public QWidget
{
	Q_OBJECT

public:
	enum ShowMode
	{
		ModeTopBottom,
		ModeLeftRight
	};

public:
	explicit SimpleVideoWidget(QWidget *parent = 0);
	virtual ~SimpleVideoWidget();

	QSize selfImageSize() const;
	QImage selfCurFrame() const;

	QSize otherImageSize() const;
	QImage otherCurFrame() const;

	void setShowMode(ShowMode mode);
	ShowMode showMode() const;

	void setTopBottomInvert(bool invert);
	bool topBottomInvert() const;

	void setSelfHide(bool hide);
	bool isSelfHide() const;

	void setOtherHide(bool hide);
	bool isOtherHide() const;

	void setSelfNameAvatar(const QString &name, const QPixmap &avatar);
	void setOtherNameAvatar(const QString &name, const QPixmap &avatar);

Q_SIGNALS:
	void videoDoubleClicked();
	void videoHasEvent();

public Q_SLOTS:
	void onOtherUpdated(const QImage &frame);
	void onOtherSizeChanged(const QSize &s);

	void onSelfUpdated(const QImage &frame);
	void onSelfSizeChanged(const QSize &s);

	void showSelfBottomMargin();
	void hideSelfBottomMargin();

protected:
	void paintEvent(QPaintEvent *e);
	void resizeEvent(QResizeEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);

private:
	void drawBigFrame(QPainter &painter);
	void drawSmallFrame(QPainter &painter);
	void drawHalfFrame(QPainter &painter);
	void drawFrameAtCenter(QPainter &painter, const QImage &frame, const QRect &sourceRect, const QSize &targetSize);
	void drawAvatarNameAtCenter(QPainter &painter, const QPixmap &avatar, const QString &name, const QRect &sourceRect);

private:
	QPixmap *m_doubleBuffer;
	QSize    m_selfImageSize;
	QImage   m_selfCurFrame;
	QSize    m_otherImageSize;
	QImage   m_otherCurFrame;
	ShowMode m_showMode;
	bool     m_topBottomInvert;
	bool     m_otherHide;
	bool     m_selfHide;
	int      m_selfBottomMargin;
	QRect    m_smallFrameRect;
	bool     m_smallHovered;
	bool     m_smallPressed;
	QPixmap  m_selfAvatar;
	QString  m_selfName;
	QPixmap  m_otherAvatar;
	QString  m_otherName;
};

#endif // SIMPLEVIDEOWIDGET_H
