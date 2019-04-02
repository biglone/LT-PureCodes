#ifndef SELECTCONTACTWIDGET_H
#define SELECTCONTACTWIDGET_H

#include <QWidget>
#include <QMap>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QItemDelegate>
#include <QHeaderView>
#include "editfiltersourcedelegate.h"

namespace Ui {class SelectContactWidget;};
class SelectContactFilterProxyModel;
class SelectContactDestSortProxyModel;
class QStandardItem;
class QStandardItemModel;
class SelectContactSourceHV;

//////////////////////////////////////////////////////////////////////////
// CLASS SelectContactWidget
class SelectContactWidget : public QWidget, public EditFilterSourceDelegate
{
	Q_OBJECT

public:
	enum SelectSource
	{
		SelectRoster = 0x01,
		SelectOS = 0x02,
		SelectAll = (SelectRoster|SelectOS)
	};

public:
	SelectContactWidget(QWidget *parent = 0);
	~SelectContactWidget();

	/*
	rosterFilterIds:      roster model中需要去除的id
	osFilterIds:          os model中需要去除的id
	preAddIds:            预先需要添加的id
	fixedIds:             如果这些id被添加了，则不允许移除
	includeStrangerGroup: 是否包含陌生人组
	*/
	void init(SelectSource source = SelectAll, 
			  const QStringList &rosterFilterIds = QStringList(),
			  const QStringList &osFilterIds = QStringList(),
			  const QStringList &preAddIds = QStringList(),
			  const QStringList &fixedIds = QStringList(),
			  int maxSelect = 0);

	QStringList selectionIds() const;

	void setSelectSource(SelectSource source);
	SelectSource getSelectSource() const;

	int maxSelect() const;
	void setMaxSelect(int max);

	void clearDest();

	void setHeaderVisible(SelectContactSourceHV *headerView, bool visible);

public: // FROM EditFilterSourceDelegate
	virtual QStringList getRosterIds();
	virtual QStringList getOsWids();

public Q_SLOTS:
	void setSkin();

Q_SIGNALS:
	void selectionChanged();

private Q_SLOTS:

	void sourceViewChanged();

	void selectAll();
	void deselectAll();
	void remove();
	void removeAll();
	void remove(const QString &id);
	void onFilterChanged(const QString &newFilter, const QString &oldFilter);
	void onFilterGainFocus();

	void destClicked(const QModelIndex &index);
	void rosterSourceClicked(const QModelIndex &index);
	void osSourceClicked(const QModelIndex &index);

	void rosterTreeViewScrolled();
	void osTreeViewScrolled();

	void onItemToggled(QStandardItem *item, bool checked);

	void onSelectionChanged();

	void onOsChildrenAdded(const QString &groupId, const QStringList &childGroupIds);

	// edit filter related
	void editFilterChanged(const QString &filterText);
	void editFilterGainFocus();
	void editFilterSelected(const QString &id, int source, const QString &wid);

private:
	void initRosterSourceModel();
	void initOSSourceModel();
	void initDestModel();
	void initUI();

	void doRosterAdd(QStandardItem *selItem);
	void doOsAdd(QStandardItem *selItem);
	void doRemove(QStandardItem *selItem);
	void doRosterRemove(QStandardItem *selItem);
	void doOsRemove(QStandardItem *selItem);

	void addDestItem(QStandardItem *sourceItem, SelectSource selectSource);
	void removeDestItem(QStandardItem *sourceItem, SelectSource selectSource);

	void checkToAddRoster(const QString &groupName);

	// return true: need to add from os model; else added before
	bool checkToAddOsContact(const QString &groupId);
	
	int  rosterGroupChildCount(const QStringList &uids, const QStringList &subIds);
	int  osGroupChildCount(const QStringList &wids, const QStringList &subIds);
	void checkRosterGroupChecked(QStandardItem *childItem);
	void checkOsGroupChecked(QStandardItem *childItem);
	void selectRosterGroupChildren(QStandardItem *parentItem, bool select = true);
	void selectOsGroupChildren(QStandardItem *parentItem, bool select = true);
	QStandardItem *createSelectItem(bool checked, int type, const QString &id, const QString &name, int index, int groupIndex, const QString &groupName);
	QString wid2Uid(const QString &wid);
	QStringList uid2Wid(const QString &uid);
	QString makeDestSourceId(const QString &uid, SelectSource selectSource);
	bool hasRosterSelect() const;
	bool hasOsContactSelect() const;
	SelectSource selectSourceFromDestSourceId(const QString &destSourceId);
	
private:
	Ui::SelectContactWidget *ui;

	SelectSource m_selectSource;

	QStandardItemModel *m_sourceRosterModel;
	QStandardItemModel *m_sourceOsModel;
	QStandardItemModel *m_destModel;

	QMap<QString, QStandardItem *> m_rosterGroupItems;
	QMap<QString, QStandardItem *> m_rosterContactItems;

	QMap<QString, QStandardItem *> m_osGroupItems;
	QMap<QString, QStandardItem *> m_osContactItems;

	QMap<QString, QStandardItem *>      m_destContactItems;
	QMap<QString, QStandardItem *>      m_destSourceItems; // <id == item>

	QStringList m_rosterFilterIds;
	QStringList m_osFilterIds;
	QStringList m_preAddIds;
	QStringList m_fixedIds;
	bool        m_includeStrangerGroup;
	int         m_maxSelect;

	SelectContactSourceHV *m_rosterHeaderView;
	SelectContactSourceHV *m_osHeaderView;
};

//////////////////////////////////////////////////////////////////////////
// CLASS SelectContactItemDelegate
class SelectContactItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit SelectContactItemDelegate(SelectContactWidget::SelectSource selectType, QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

Q_SIGNALS:
	void itemToggled(QStandardItem *item, bool checked);
	void clicked(const QModelIndex &index);

protected:
	virtual void drawGroupItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void drawContactItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	QPixmap                           m_avatar;
	QPixmap                           m_uncheckedNormal;
	QPixmap                           m_uncheckedDisabled;
	QPixmap                           m_checkedNormal;
	QPixmap                           m_checkedDisabled;
	QPixmap                           m_partiallyCheckedNormal;
	QPixmap                           m_partiallyCheckedDisabled;
	QPixmap                           m_groupExpand;
	QPixmap                           m_groupCollapse;
	SelectContactWidget::SelectSource m_sourceType;
};

class SelectRosterContactItemDelegate : public SelectContactItemDelegate
{
public:
	explicit SelectRosterContactItemDelegate(SelectContactWidget::SelectSource selectType, QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
	virtual void drawGroupItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

//////////////////////////////////////////////////////////////////////////
// CLASS SelectContactDestItemDelegate
class SelectContactDestItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	explicit SelectContactDestItemDelegate(QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

Q_SIGNALS:
	void delButtonClicked(const QString &id);

private slots:
	void onDelButtonClicked();

private:
	void drawContactItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	QPixmap                  m_avatar;
	QMap<QString, QWidget *> m_delButtons;
	QString                  m_lastId;
};

//////////////////////////////////////////////////////////////////////////
// class SelectContactSourceHV
class SelectContactSourceHV : public QHeaderView
{
	Q_OBJECT

public:
	SelectContactSourceHV(SelectContactWidget *widget, QStandardItemModel *sourceModel, QWidget *parent = 0);

	void setGroupItem(QStandardItem *item);

private slots:
	void onSectionClicked(int index);

private:
	QStandardItemModel            *m_sourceModel;
	QStandardItem                 *m_groupItem;
	SelectContactWidget           *m_widget;
};

#endif // SELECTCONTACTWIDGET_H
