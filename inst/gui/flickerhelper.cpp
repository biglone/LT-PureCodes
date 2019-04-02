#include "flickerhelper.h"
#include <QWidget>
#include <QTimer>

const int Default_Flicker_Interval = 200; // 200ms

FlickerHelper::FlickerHelper(QObject *parent)
	: QObject(parent), m_flickerInterval(Default_Flicker_Interval), m_flickerIndex(0)
{
	m_timer = new QTimer(this);
	m_timer->setSingleShot(false);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(flickerTimeout()));
}

FlickerHelper::~FlickerHelper()
{
}

void FlickerHelper::setFlickerInterval(int mSecs)
{
	m_flickerInterval = mSecs;
}

int FlickerHelper::flickerInterval() const
{
	return m_flickerInterval;
}

void FlickerHelper::addFlickerWidget(IFlickerable *flickerWidget)
{
	if (!m_flickerWidgets.contains(flickerWidget))
		m_flickerWidgets.append(flickerWidget);
}

void FlickerHelper::removeFlickerWidget(IFlickerable *flickerWidget)
{
	if (m_flickerWidgets.contains(flickerWidget))
		m_flickerWidgets.removeAll(flickerWidget);
}

void FlickerHelper::addFlickerItem(const QString &id, bean::MessageType msgType)
{
	FlickerItem item(id, msgType);
	if (!m_flickerItems.contains(item))
	{
		m_flickerItems.append(item);

		if (m_flickerItems.count() == 1)
			startFlickering();
	}
}

void FlickerHelper::removeFlickerItem(const QString &id, bean::MessageType msgType)
{
	FlickerItem item(id, msgType);
	if (m_flickerItems.contains(item))
	{
		// check which one is flicking now
		QList<IFlickerable *> updateWidgets;
		foreach (IFlickerable *flickerWidget, m_flickerWidgets)
		{
			if (flickerWidget->containsFlickerItem(id, msgType))
				updateWidgets.append(flickerWidget);
		}

		m_flickerItems.removeAll(item);

		// update the widgets
		foreach (IFlickerable *updateWidget, updateWidgets)
		{
			updateWidget->doUpdate();
		}

		if (m_flickerItems.isEmpty())
			stopFlickering();
	}
}

bool FlickerHelper::containsFlickerItem(const QString &id, bean::MessageType msgType)
{
	FlickerItem item(id, msgType);
	return m_flickerItems.contains(item);
}

void FlickerHelper::clearFlickerWidgets()
{
	m_flickerWidgets.clear();
}

void FlickerHelper::clearFlickerItems()
{
	if (!m_flickerItems.isEmpty())
	{
		m_flickerItems.clear();

		// update all widgets
		foreach (IFlickerable *updateWidget, m_flickerWidgets)
		{
			updateWidget->doUpdate();
		}
	}

	stopFlickering();
}

int FlickerHelper::flickerIndex() const
{
	return m_flickerIndex;
}

void FlickerHelper::flickerTimeout()
{
	if (!m_flickerWidgets.isEmpty() && isFlickering())
	{
		if (m_flickerIndex == INT_MAX)
			m_flickerIndex = 0;
		m_flickerIndex++;

		QList<IFlickerable *> flickerWidgets = m_flickerWidgets;
		foreach (FlickerItem item, m_flickerItems)
		{
			if (flickerWidgets.isEmpty())
				break;

			for (int i = flickerWidgets.count()-1; i >= 0; i--)
			{
				IFlickerable *flickerWidget = flickerWidgets[i];
				if (flickerWidget->containsFlickerItem(item.id, item.msgType)) // check if this widget need to flick
				{
					// flick this widget
					flickerWidget->doUpdate();

					// remove from flick widgets
					flickerWidgets.removeAt(i);
				}
			}
		}
	}
}

bool FlickerHelper::isFlickering() const
{
	return m_timer->isActive();
}

void FlickerHelper::startFlickering()
{
	if (!m_timer->isActive())
		m_timer->start(m_flickerInterval);
}

void FlickerHelper::stopFlickering()
{
	if (m_timer->isActive())
		m_timer->stop();
}
