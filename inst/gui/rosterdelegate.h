#ifndef ROSTERDELEGATE_H
#define ROSTERDELEGATE_H

#include <QItemDelegate>
#include <QBitmap>

class RosterModel;
class FlickerHelper;
class QStandardItem;

class RosterDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	enum AvatarType
	{
		BigAvatar = 1,
		SmallAvatar = 2
	};

public:
	explicit RosterDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setRosterModel(RosterModel *rosterModel) {m_rosterModel = rosterModel;}
	void setFlickerHelper(FlickerHelper *flickerHelper) {m_flickerHelper = flickerHelper;}
	AvatarType avatarType() const {return m_avatarType;}
	void setAvatarType(AvatarType t) {m_avatarType = t;}

private:
	void drawGroupItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawBigAvatarRosterItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawSmallAvatarRosterItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	bool isGroupFlickering(QStandardItem *groupItem) const;
	bool isContactFlickering(QStandardItem *contactItem) const;

private:
	QBitmap        m_avatarMask;
	RosterModel   *m_rosterModel;
	FlickerHelper *m_flickerHelper;
	AvatarType     m_avatarType;
};

#endif // ROSTERDELEGATE_H
