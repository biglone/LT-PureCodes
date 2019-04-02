#ifndef ORGANIZATIONDELEGATE_H
#define ORGANIZATIONDELEGATE_H

#include <QItemDelegate>

class OrgStructModel;
class FlickerHelper;
class OrgStructContactItem;
class OrgStructGroupItem;

class OrganizationDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit OrganizationDelegate(QObject *parent = 0);
	virtual ~OrganizationDelegate();
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setOrgModel(OrgStructModel *orgModel) {m_orgModel = orgModel;}
	void setFlickerHelper(FlickerHelper *flickerHelper) {m_flickerHelper = flickerHelper;}

	static const int kmargin = 16;

private:
	void drawGroupItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	void drawContactItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	bool isGroupFlickering(OrgStructGroupItem *groupItem) const;
	bool isContactFlickering(OrgStructContactItem *contactItem) const;

private:
	QImage          m_openBranchImg;
	QImage          m_closeBranchImg;
	QPixmap         m_contactPixmap;

	OrgStructModel *m_orgModel;
	FlickerHelper  *m_flickerHelper;
};

#endif // ORGANIZATIONDELEGATE_H
