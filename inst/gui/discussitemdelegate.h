#ifndef DISCUSSITEMDELEGATE_H
#define DISCUSSITEMDELEGATE_H

#include <QItemDelegate>

class DiscussModel;
class FlickerHelper;

class DiscussItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit DiscussItemDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setDiscussModel(DiscussModel *discussModel) {m_discussModel = discussModel;}
	void setFlickerHelper(FlickerHelper *flickerHelper) {m_flickerHelper = flickerHelper;}

private:
	DiscussModel *m_discussModel;
	FlickerHelper *m_flickerHelper;
};

#endif // DISCUSSITEMDELEGATE_H
