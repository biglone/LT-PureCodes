#ifndef FLICKERHELPER_H
#define FLICKERHELPER_H

#include <QObject>
#include <QList>
#include "flickerable.h"

class QTimer;

class FlickerHelper : public QObject
{
	Q_OBJECT

public:
	FlickerHelper(QObject *parent = 0);
	~FlickerHelper();

	// set & get interval
	void setFlickerInterval(int mSecs);
	int flickerInterval() const;

	// add & remove widget
	void addFlickerWidget(IFlickerable *flickerWidget);
	void removeFlickerWidget(IFlickerable *flickerWidget);

	// add & remove flicker item
	void addFlickerItem(const QString &id, bean::MessageType msgType);
	void removeFlickerItem(const QString &id, bean::MessageType msgType);

	bool containsFlickerItem(const QString &id, bean::MessageType msgType);

	// clear flicker widgets and items
	void clearFlickerWidgets();
	void clearFlickerItems();

	// get flicker index
	int flickerIndex() const;

private slots:
	// time out
	void flickerTimeout();

private:
	bool isFlickering() const;
	void startFlickering();
	void stopFlickering();

private:
	struct FlickerItem
	{
		QString     id;
		bean::MessageType msgType;

		FlickerItem(const QString mId, bean::MessageType mType) : id(mId), msgType(mType) {}
		bool operator==(const FlickerItem &other) const
		{
			if (this->id == other.id && this->msgType == other.msgType)
			{
				return true;
			}
			return false;
		}
	};

private:
	QList<IFlickerable *> m_flickerWidgets;
	QList<FlickerItem>    m_flickerItems;
	QTimer                *m_timer;
	int                   m_flickerInterval;
	int                   m_flickerIndex;
};

#endif // FLICKERHELPER_H
