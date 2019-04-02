#ifndef LASTCONTACTITEMDELEGATE_H
#define LASTCONTACTITEMDELEGATE_H

#include <QItemDelegate>
#include <QMap>
#include "bean/bean.h"

class LastContactModel;
class FlickerHelper;

class LastContactItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	enum InterphoneState
	{
		InterphoneNone,
		InterphoneNotIn,
		InterphoneIn
	};

public:
	explicit LastContactItemDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setLastContactModel(LastContactModel *lastContactModel) {m_lastContactModel = lastContactModel;}
	void setFlickerHelper(FlickerHelper *flickerHelper) {m_flickerHelper = flickerHelper;}
	
	void setInterphoneState(const QString &interphoneId, InterphoneState state);
	InterphoneState interphoneState(const QString &interphoneId) const;
	void clearInterphoneStates();

private:
	LastContactModel *m_lastContactModel;
	FlickerHelper    *m_flickerHelper;

	QMap<QString, InterphoneState> m_interphoneStates;
};

#endif // LASTCONTACTITEMDELEGATE_H
