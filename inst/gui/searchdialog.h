#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include "framelessdialog.h"
#include "searchmanager.h"
#include "subscriptiondetail.h"
#include "subscriptionmsg.h"
#include <QNetworkAccessManager>

namespace Ui {class SearchDialog;};
class QModelIndex;

class SearchDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum SearchType
	{
		SearchPeople,
		SearchSubscription
	};

public:
	static SearchDialog *getInstance();
	SearchDialog(QWidget *parent = 0);
	~SearchDialog();

	void setSearchType(SearchType type);

Q_SIGNALS:
	void addFriendRequest(const QString &id, const QString &name);
	void showMaterial(const QString &id);
	void subscriptionSubscribed(const SubscriptionDetail &subscription, const SubscriptionMsg &msg);
	void subscribeSucceed(const QString &subscriptionId);

public slots:
	virtual void setSkin();

private slots:
	void onSearchPeopleFinished(bool ok);
	void onSearchSubscriptionFinished(bool ok, int currentPage, int pageSize, int rowCount, 
		int totalPage, const QList<SubscriptionDetail> &subscriptions);
	void on_tabSearchPeople_clicked();
	void on_tabSearchSubscription_clicked();
	void on_btnSearchPeople_clicked();
	void on_btnSearchSubscription_clicked();
	void onTableViewItemClicked(const QModelIndex &index);
	void on_pBtnBegin_clicked();
	void on_pBtnPrevious_clicked();
	void on_pBtnNext_clicked();
	void on_pBtnEnd_clicked();
	// void on_leditPage_returnPressed();
	void onAddFriend(const QString &id);
	void subscribe(const SubscriptionDetail &subscription);
	void onSubscribeFinished(bool ok, const QString &subscriptionId, const SubscriptionMsg &msg);

protected:
	void keyPressEvent(QKeyEvent *e);

private:
	void initUI();
	void initSignals();
	void setSearchText(const QString &keyword);
	void setSearchPeopleResult(SearchManager::SearchResult *searchResult);
	void setSearchSubscriptionResult(const QList<SubscriptionDetail> &subscriptions);
	void setPageInfo();
	void searchPeople(const QString &name, int page, int size);
	void searchSubscription(const QString &keyword, int page, int size);

private:
	Ui::SearchDialog *ui;

	int m_index;
	int m_pages;
	int m_count;
	int m_tableItemHeight;

	QMap<QString, SubscriptionDetail> m_subscribeMap;

	QNetworkAccessManager m_networkAccessManager;

	static SearchDialog *s_instance;
};

#endif // SEARCHDIALOG_H
