#include "expandtabwidget.h"

TabPushButton::TabPushButton(QWidget *parent):
    QPushButton(parent)
{
    setCheckable(true);
    setChecked(false);
    connect(this, SIGNAL(toggled(bool)), SLOT(onToggled(bool)));
    connect(this, SIGNAL(clicked()), SLOT(onClicked()));

	setFixedHeight(36);

	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);
}

void TabPushButton::setBorderStyle(const WidgetBorderStyle& style)
{
    setStyleSheet(QString("QPushButton{"
                          "border-top: %1px;"
                          "border-left: %2px;"
                          "border-right: %3px;"
                          "border-bottom: %4px;"
                          "border-image: url(%5);"
                          "}"

                          "QPushButton:hover{"
                          "border-image: url(%6);"
                          "}"
                          "QPushButton:checked{"
                          "border-image: url(%7);"
                          "}"
                          ).arg(style.border.top)
                  .arg(style.border.left)
                  .arg(style.border.right)
                  .arg(style.border.bottom)
                  .arg(style.urls.normal)
                  .arg(style.urls.hover)
                  .arg(style.urls.selected)
                  );
}

void TabPushButton::setTabButtonIcon(const QString& iconNormal, const QString& iconSelected, const QString& iconHover)
{
    m_iconNormal = QPixmap(iconNormal);
    m_iconSelected = QPixmap(iconSelected);
	m_iconHover = QPixmap(iconHover);
	
    onToggled(isChecked());
}

void TabPushButton::enterEvent(QEvent *e)
{
	if (!isChecked() && !m_iconHover.isNull())
	{
		QIcon icon(m_iconHover);
		setIcon(icon);
	}
}

void TabPushButton::leaveEvent(QEvent *e)
{
	if (!isChecked())
	{
		QIcon icon(m_iconNormal);
		setIcon(icon);
	}
	else
	{
		QIcon icon(m_iconSelected);
		setIcon(icon);
	}
}

void TabPushButton::onToggled (bool checked)
{
    if (checked)
    {
		QIcon icon(m_iconSelected);
		setIcon(icon);
    }
    else
    {
		QIcon icon(m_iconNormal);
		setIcon(icon);
    }
}

void TabPushButton::onClicked()
{
    setChecked(true);
}


ExpandTabWidget::ExpandTabWidget(QWidget *parent) :
    QWidget(parent)
{
    m_stackedWidget = new SlidingStackedWidget(this);

    m_layoutMain = new QVBoxLayout(this);
    m_layoutMain->setSpacing(0);
    m_layoutMain->setContentsMargins(0,0,0,0);
    m_tabBar = new QWidget(this);
    m_tabBar->setObjectName("tabBar");
    m_tabBar->setLayout(new QHBoxLayout(m_tabBar));
    m_tabBar->layout()->setSpacing(0);
    m_tabBar->layout()->setContentsMargins(0,0,0,0);
    m_layoutMain->addWidget(m_tabBar, 0);
    m_layoutMain->addWidget(m_stackedWidget, 1);
}

void ExpandTabWidget::setTabBarMargin(int left, int top, int right, int bottom)
{
	m_tabBar->layout()->setContentsMargins(left, top, right, bottom);
}

void ExpandTabWidget::setTabBarBackground(const QString& background)
{
	m_tabBar->setStyleSheet(QString(
		"QWidget#tabBar {"
		"	background-image: url(%1);"
		"}"
		).arg(background));
}

void ExpandTabWidget::setTabButtonBorderStyle(const WidgetBorderStyle& style)
{
    m_tabButtonStyle = style;
    foreach(TabPushButton* btn, m_tabButtonsMap.keys())
    {
        btn->setBorderStyle(m_tabButtonStyle);
    }
}

void ExpandTabWidget::setTabButtonIcon(QWidget* widget, const QString& iconNormal, const QString& iconSelected, const QString &iconHover)
{
    if (m_tabButtonsMap.values().contains(widget))
    {
		m_tabButtonsMap.key(widget)->setTabButtonIcon(iconNormal, iconSelected, iconHover);
    }
}

void ExpandTabWidget::setTabButtonSeperator(const QString& seperator)
{
	m_seperatorPixmap = QPixmap(seperator);
	if (!m_seperatorPixmap.isNull())
	{
		foreach(QLabel* sepLabel, m_seperatorLabels)
		{
			if (sepLabel)
			{
				sepLabel->setPixmap(m_seperatorPixmap);
				sepLabel->setFixedSize(m_seperatorPixmap.size());
			}
		}
	}
	else
	{
		foreach(QLabel* sepLabel, m_seperatorLabels)
		{
			if (sepLabel)
			{
				sepLabel->setFixedSize(QSize(0, 0));
			}
		}
	}
}

void ExpandTabWidget::setTabText(QWidget* widget, const QString& text)
{
	if (m_tabButtonsMap.values().contains(widget))
	{
		m_tabButtonsMap.key(widget)->setText(text);
	}
}

void ExpandTabWidget::addTab(QWidget *widget, int order, const QString &tp)
{
	// insert widget
    int index = m_stackedWidget->count();
    if(m_stackedWidget->count() > 0)
    {
        for(int i=m_stackedWidget->count()-1; i>=0; i--)
        {
            QWidget* addedWidget = m_stackedWidget->widget(i);
            if(m_tabWidgetOrders.contains(addedWidget) &&
                    m_tabWidgetOrders[addedWidget] <= order)
            {
                index = i+1;
                break;
            }
            if(i == 0)
                index = 0;
        }
    }
    m_stackedWidget->insertWidget(index, widget);
    m_tabWidgetOrders[widget] = order;

	// insert tab button first
	int tabInsertIndex = index*2;
	QHBoxLayout* tabLayout = qobject_cast<QHBoxLayout*>(m_tabBar->layout());

	if(index > 0) // not the last tab, need add seperator before tab button
	{
		int sepInsertIndex = tabInsertIndex-1;
		QLabel *label = new QLabel(this);
		label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
		label->setPixmap(m_seperatorPixmap);
		tabLayout->insertWidget(sepInsertIndex, label);
		m_seperatorLabels.insert(sepInsertIndex, label);

		TabPushButton* btn = new TabPushButton(this);
		btn->setFocusPolicy(Qt::NoFocus);
		btn->setIconSize(QSize(32,32));
		if(!tp.isEmpty())
			btn->setToolTip(tp);
		btn->setBorderStyle(m_tabButtonStyle);
		tabLayout->insertWidget(tabInsertIndex, btn);
		m_tabButtonsMap[btn] = widget;
		connect(btn, SIGNAL(clicked(bool)), SLOT(onTabButtonClicked(bool)));
		m_seperatorLabels.insert(tabInsertIndex, 0);
	}
	else if (index == 0 && index != m_stackedWidget->count()-1) // is the first place, add the seperator after tab button
	{
		TabPushButton* btn = new TabPushButton(this);
		btn->setFocusPolicy(Qt::NoFocus);
		btn->setIconSize(QSize(32,32));
		if(!tp.isEmpty())
			btn->setToolTip(tp);
		btn->setBorderStyle(m_tabButtonStyle);
		tabLayout->insertWidget(tabInsertIndex, btn);
		m_tabButtonsMap[btn] = widget;
		connect(btn, SIGNAL(clicked(bool)), SLOT(onTabButtonClicked(bool)));
		m_seperatorLabels.insert(tabInsertIndex, 0);

		int sepInsertIndex = tabInsertIndex+1;
		QLabel *label = new QLabel(this);
		label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
		label->setPixmap(m_seperatorPixmap);
		tabLayout->insertWidget(sepInsertIndex, label);
		m_seperatorLabels.insert(sepInsertIndex, label);
	}
	else // index == 0 && m_stackedWidget->count() == 1 : only add tab self
	{
		TabPushButton* btn = new TabPushButton(this);
		btn->setFocusPolicy(Qt::NoFocus);
		btn->setIconSize(QSize(32,32));
		if(!tp.isEmpty())
			btn->setToolTip(tp);
		btn->setBorderStyle(m_tabButtonStyle);
		tabLayout->insertWidget(tabInsertIndex, btn);
		m_tabButtonsMap[btn] = widget;
		connect(btn, SIGNAL(clicked(bool)), SLOT(onTabButtonClicked(bool)));
		m_seperatorLabels.insert(tabInsertIndex, 0);
	}
	
    setCurrentIndex(0);
}

void ExpandTabWidget::setCurrentIndex(int index)
{
    if (index <0 || index >= m_stackedWidget->count())
    {
        return;
    }
    QWidget* widget = m_stackedWidget->widget(index);
    foreach (TabPushButton* btn, m_tabButtonsMap.keys()) {
        if(m_tabButtonsMap[btn] != widget)
        {
            btn->setChecked(false);
        }
        else
        {
            btn->setChecked(true);
        }
    }
    m_stackedWidget->setCurrentIndex(index);
    emit currentIndexChanged(index);
}

void ExpandTabWidget::setCurrentWidget(QWidget* widget)
{
	setCurrentIndex(m_stackedWidget->indexOf(widget));
}

int ExpandTabWidget::currentIndex() const
{
    return m_stackedWidget->currentIndex();
}

void ExpandTabWidget::onTabButtonClicked(bool bChecked)
{
    TabPushButton* btn = qobject_cast<TabPushButton*>(sender());
    if(btn && bChecked)
    {
        setCurrentIndex(m_stackedWidget->indexOf(m_tabButtonsMap[btn]));
    }
}

QWidget* ExpandTabWidget::widget(int index) const
{
    return m_stackedWidget->widget(index);
}

QWidget* ExpandTabWidget::currentWidget() const
{
    return m_stackedWidget->currentWidget();
}

void ExpandTabWidget::setTabButtonIconSize(const QSize &size)
{
    foreach (TabPushButton* btn, m_tabButtonsMap.keys()) {
        btn->setIconSize(size);
    }
}

void ExpandTabWidget::setTabVisible(int index, const bool bVisible /* = true */)
{
	if (index >= 0 && index < m_stackedWidget->count())
	{
		QWidget* widget = m_stackedWidget->widget(index);
		TabPushButton* btn = qobject_cast<TabPushButton*>(m_tabButtonsMap.key(widget));
		if (btn->isVisible() != bVisible)
		{
			btn->setVisible(bVisible);
			
			// check if this tab is the last one, if not last, need to set the seperator visibility
			if (m_stackedWidget->count() > 1)
			{
				int tabButtonIndex = 2*index;
				if (tabButtonIndex == 0)
				{
					if (m_seperatorLabels.count() > 1 && m_seperatorLabels[1] != 0)
						m_seperatorLabels[1]->setVisible(bVisible);
				}
				else
				{
					if (m_seperatorLabels.count() > tabButtonIndex && m_seperatorLabels[tabButtonIndex-1] != 0 )
						m_seperatorLabels[tabButtonIndex-1]->setVisible(bVisible);
				}
			}

			if (!bVisible)
			{
				if ((index+1) >= m_stackedWidget->count())
				{
					setCurrentIndex(0);
				}
				else
				{
					setCurrentIndex(index + 1);
				}
			}
		}
	}
}

void ExpandTabWidget::setTabVisible(QWidget* widget,const bool bVisible /* = true */)
{
	int index = m_stackedWidget->indexOf(widget);
	setTabVisible(index, bVisible);
}

bool ExpandTabWidget::isTabVisible(int index)
{
	if (index >= 0 && index < m_stackedWidget->count())
	{
		QWidget* widget = m_stackedWidget->widget(index);
		TabPushButton* btn = qobject_cast<TabPushButton*>(m_tabButtonsMap.key(widget));
		return btn->isVisible();
	}
	return false;
}

bool ExpandTabWidget::isTabVisible(QWidget* widget)
{
	int index = m_stackedWidget->indexOf(widget);
	return isTabVisible(index);
}