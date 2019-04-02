#ifndef EDITFILTERWIDGET_H
#define EDITFILTERWIDGET_H

#include <QWidget>

namespace Ui {class EditFilterWidget;};
class EditFilterTreeView;
class EditFilterListView;
class EditFilterSourceDelegate;

extern const int SELECT_SOURCE_ROSTER;
extern const int SELECT_SOURCE_OS;
extern const int SELECT_SOURCE_GROUP;
extern const int SELECT_SOURCE_DISCUSS;
extern const int SELECT_SOURCE_SUBSCRIPTION;

class EditFilterWidget : public QWidget
{
	Q_OBJECT

public:
	enum SelectMode
	{
		DoubleClick, // default
		SingleClick
	};

public:
	EditFilterWidget(QWidget *parent = 0);
	~EditFilterWidget();

	EditFilterTreeView *treeViewSearch() const;
	EditFilterListView *listViewSearch() const;

	void setSelectMode(SelectMode mode);

	void setCurrentIndex(int index);

	void addEditFilterCompleter(bool addRoster = true, bool addOs = true, bool addGroup = true, bool addDiscuss = true, bool addSubscription = true);
	void editFilterChanged(const QString &filterText);

	void setTipText(const QString &tipText);
	QString tipText() const;
	void setTipTextVisible(bool visible);

	void setSourceDelegate(EditFilterSourceDelegate *sourceDelegate);

signals:
	void chat(const QString &id);
	void groupChat(const QString &id);
	void discussChat(const QString &id);
	void subscriptionChat(const QString &id);
	void viewMaterial(const QString &id);

	/* source: 0-roster 1-organization 2-group 3-discuss */
	void selectSearchItem(const QString &id, int source, const QString &wid = QString());

private slots:
	void viewAll(int type);
	void on_labelReturn_clicked();
	void on_labelPageUp_clicked();
	void on_labelPageDown_clicked();

private:
	void searchInAll(const QString &searchStr, int &rosterCount, int &osCount, int &groupCount, int &discussCount, 
		             int &subscriptionCount, int maxCount = 100);
	bool getMatchName(const QString &name, const QString &searchStr, QString &matchPart, const QString &id = QString());
	
	void searchRoster(const QString &searchStr);
	void searchOs(const QString &searchStr);
	void searchGroup(const QString &searchStr);
	void searchDiscuss(const QString &searchStr);
	void searchSubscription(const QString &searchStr);
	void onSearchFinished();

	QStringList getRosterIds();
	QStringList getOsWids();
	QStringList getGroupIds();
	QStringList getDiscussIds();
	QStringList getSubscriptionIds();

private:
	Ui::EditFilterWidget *ui;

	QString m_searchString;
	int m_totalCount;
	int m_pageIndex;
	int m_pageOfCount;

	QString m_tipText;

	EditFilterSourceDelegate *m_sourceDelegate;
};

#endif // EDITFILTERWIDGET_H
