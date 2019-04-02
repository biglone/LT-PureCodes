#ifndef AUTOTIPWIDGET_H
#define AUTOTIPWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>

namespace Ui {class AutoTipWidget;};

class AutoTipWidget : public QWidget
{
	Q_OBJECT

public:
	AutoTipWidget(QWidget *parent = 0);
	~AutoTipWidget();

	void setTipPixmap(const QPixmap &pixmap);
	void setTipText(const QString &text);
	void setTipTextColor(const QColor &color);
	void setBackgroundColor(const QColor &color);
	void setCanClose(bool canClose);

public slots:
	void autoShow();
	void stopShow();

protected:
	void paintEvent(QPaintEvent *ev);

private slots:
	void onShowTimer();

private:
	Ui::AutoTipWidget *ui;
	QTimer m_autoHideTimer;
	int    m_showSeconds;
	QColor m_backgroundColor;
};

#endif // AUTOTIPWIDGET_H
