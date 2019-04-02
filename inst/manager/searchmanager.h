#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <QObject>
#include "pmclient/PmClientInterface.h"
#include <QString>
#include <QStringList>
#include <QList>

class HttpPool;

class SearchManager : public QObject
{
	Q_OBJECT
	
public:
	struct SearchItem
	{
		QString m_id;
		QString m_name;
		int     m_sex;
		QString m_depart;
		QString m_phone;
		int     m_userState;
		int     m_frozen;

		SearchItem(const QString &id, const QString &name, int sex, const QString &depart, const QString &phone)
		{
			m_id = id;
			m_name = name;
			m_sex = sex;
			m_depart = depart;
			m_phone = phone;
		}
	};

	struct SearchResult
	{
		QString m_searchName;
		QString m_searchSex;
		QString m_searchPhone;
		int     m_currentPage;
		int     m_pageSize;
		int     m_count;
		QList<SearchManager::SearchItem> m_items;

		SearchResult()
		{
			m_currentPage = 0;
			m_pageSize = 6;
			m_count = 0;
		}

		SearchResult(const SearchResult &other)
		{
			m_searchName = other.m_searchName;
			m_searchSex = other.m_searchSex;
			m_searchPhone = other.m_searchPhone;
			m_currentPage = other.m_currentPage;
			m_pageSize = other.m_pageSize;
			m_count = other.m_count;
			m_items = other.m_items;
		}

		SearchResult& operator=(const SearchResult &other)
		{
			if (&other != this)
			{
				m_searchName = other.m_searchName;
				m_searchSex = other.m_searchSex;
				m_searchPhone = other.m_searchPhone;
				m_currentPage = other.m_currentPage;
				m_pageSize = other.m_pageSize;
				m_count = other.m_count;
				m_items = other.m_items;
			}
			return *this;
		}
	};

public:
	SearchManager(QObject *parent = 0);
	~SearchManager();

public:
	void conditionSearch(const QString &name, const QString &sex, const QString &phone, int page, int size);

	SearchManager::SearchResult *searchResult() const {return m_searchResult;}
	QString lastError() const {return m_lastError;}

Q_SIGNALS:
	void searchFinished(bool ok);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	SearchResult     *m_searchResult;
	QString           m_lastError;
	HttpPool         *m_httpPool;
	QList<int>        m_searchIds;
};

#endif // SEARCHMANAGER_H
