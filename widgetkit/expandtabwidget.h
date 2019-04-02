#ifndef EXPANDTABWIDGET_H
#define EXPANDTABWIDGET_H

#include <QPushButton>
#include <QWidget>
#include "slidingstackedwidget.h"
#include <QBoxLayout>
#include "widgetborder.h"
#include <QPixmap>
#include <QLabel>
#include <QList>

#include "widgetkit_global.h"

class TabPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit TabPushButton(QWidget *parent = 0);
	void setTabButtonIcon(const QString& iconNormal, const QString& iconSelected, const QString& iconHover);
	void setBorderStyle(const WidgetBorderStyle& style);

protected:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);

public slots:
    void onToggled(bool checked);
	void onClicked();

signals:
    void selected();

private:
	QPixmap m_iconNormal;
    QPixmap m_iconSelected;
	QPixmap m_iconHover;
};

class WIDGETKIT_EXPORT ExpandTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExpandTabWidget(QWidget *parent = 0);
	void setTabBarMargin(int left, int top, int right, int bottom);
	void setTabBarBackground(const QString& background);
	void setTabButtonBorderStyle(const WidgetBorderStyle& style);
	void setTabButtonIcon(QWidget* widget, const QString& iconNormal, const QString& iconSelected, const QString& iconHover);
	void setTabButtonSeperator(const QString& seperator);
	void setTabText(QWidget* widget, const QString& text);
    void addTab(QWidget* widget, int order, const QString& tp = "");
    void setCurrentIndex(int index);
	void setCurrentWidget(QWidget* widget);
    int currentIndex() const;
    QWidget* widget(int index) const;
    QWidget* currentWidget()const;
    void setTabButtonIconSize(const QSize& size = QSize(32,32));
	void setTabVisible(int index, const bool bVisible = true);
	void setTabVisible(QWidget* widget, const bool bVisible = true);
	bool isTabVisible(int index);
	bool isTabVisible(QWidget* widget);

signals:
    void currentIndexChanged(int index);
    
public slots:
    void onTabButtonClicked(bool bChecked);

private:
    SlidingStackedWidget* m_stackedWidget;
    QWidget* m_tabBar;
    QVBoxLayout* m_layoutMain;
    QMap<TabPushButton*, QWidget*> m_tabButtonsMap;
    QMap<QWidget*, int> m_tabWidgetOrders;
	WidgetBorderStyle m_tabButtonStyle;
    QPixmap m_seperatorPixmap;
	QList<QLabel *> m_seperatorLabels;
};

#endif // EXPANDTABWIDGET_H
